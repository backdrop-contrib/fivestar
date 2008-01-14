<?php

define('FIVESTAR_COMMENT_DISABLED', 0);
define('FIVESTAR_COMMENT_OPTIONAL', 1);
define('FIVESTAR_COMMENT_REQUIRED', 2);

/**
 * Form alter specification for comments.
 */
function fivestar_comment_form_alter($form_id, &$form) {
  // Comment settings.
  if ($form_id == 'node_type_form' && isset($form['identity']['type'])) {
    $form['fivestar']['comment'] = array(
      '#type' => 'fieldset',
      '#title' => t('Comment widget'),
      '#description' => t('Enabling Fivestar for comments will display a rating widget when a user posts a comment. The rating of the comment will affect its parent content.'),
      '#weight' => 1,
    );
    $form['fivestar']['comment']['fivestar_comment'] = array(
      '#type' => 'radios',
      '#title' => t('Fivestar comment settings'),
      '#options' => array(
        FIVESTAR_COMMENT_DISABLED => t('Disabled'),
        FIVESTAR_COMMENT_OPTIONAL => t('Optional rating'),
        FIVESTAR_COMMENT_REQUIRED => t('Required rating'),
      ),
      '#default_value' => variable_get('fivestar_comment_'. $form['#node_type']->type, 0),
    );
    $form['fivestar']['comment']['fivestar_comment_preview'] = array(
      '#type' => 'item',
      '#title' => t('Comment widget preview'),
      '#value' => theme('fivestar_preview', 'compact', 'none', $form['fivestar']['fivestar_stars']['#default_value'], $form['fivestar']['comment']['fivestar_comment']['#default_value'] == 1 ? 1 : 0),
    );
    if (!$form['fivestar']['fivestar']['#default_value'] || !$form['fivestar']['comment']['fivestar_comment']['#default_value']) {
      $form['fivestar']['comment']['fivestar_comment_preview']['#value'] = theme('fivestar_preview_wrapper', '', 'comment');
    }
    else {
      $form['fivestar']['comment']['fivestar_comment_preview']['#value'] = theme('fivestar_preview_wrapper', $form['fivestar']['comment']['fivestar_comment_preview']['#value'], 'comment');
    }
  }

  // Comment form. Do not allow ratings inside of threads.
  if ($form_id == 'comment_form' && empty($form['pid']['#value'])) {
    $node = node_load($form['nid']['#value']);
    if (variable_get('fivestar_comment_'. $node->type, 0)) {
      // Splice in the fivestar right before the body.
      $new_form = array();
      foreach ($form as $key => $element) {
        if ($key == 'comment_filter') {
          $new_form['fivestar_rating'] = array(
            '#type' => 'fivestar',
            '#title' => t('Rating'),
            '#stars' => variable_get('fivestar_stars_'. $node->type, 5),
            '#allow_clear' => variable_get('fivestar_comment_'. $node->type, FIVESTAR_COMMENT_DISABLED) == FIVESTAR_COMMENT_OPTIONAL ? 1 : 0,
            '#content_id' => $node->nid,
            '#required' => variable_get('fivestar_comment_'. $node->type, FIVESTAR_COMMENT_DISABLED) == FIVESTAR_COMMENT_REQUIRED ? 1 : 0,
            '#default_value' => $form['cid']['#value'] ? fivestar_comment_load($form['cid']['#value']) : 0,
            '#description' => '&nbsp;',
            '#labels' => variable_get('fivestar_labels_'. $node->type, array()),
          );
        }
        $new_form[$key] = $element;
      }
      if ($new_form['fivestar_rating']) {
        $form = $new_form;
      }
    }
  }
}

/**
 * Implementation of hook_comment().
 */
function fivestar_comment(&$comment, $op) {
  switch ($op) {
    case 'view':
      $node = node_load($comment->nid);
      $comment->fivestar_rating = isset($comment->fivestar_rating) ? $comment->fivestar_rating : fivestar_comment_load($comment->cid);
      $comment->fivestar_view = theme('fivestar_static', $comment->fivestar_rating, variable_get('fivestar_stars_'. $node->type, 5));
      if ($comment->fivestar_rating) {
        $comment->comment = $comment->fivestar_view . $comment->comment;
      }
      break;
    case 'insert':
    case 'update':
      // Why comments are arrays here... only comment.module knows.
      fivestar_comment_set((object)$comment);
      break;
    case 'delete':
      fivestar_comment_delete($comment->cid, $comment->nid, $comment->uid);
      break;
  }
}

/**
 * Set the value of a comment rating.
 */
function fivestar_comment_set($comment) {
  // Insert/update.
  if ($comment->fivestar_rating) {
    $current_rating = fivestar_comment_load($comment->cid);
    if ($current_rating) {
      fivestar_comment_update($comment->cid, $comment->nid, $commnt->uid, $comment->fivestar_rating);
    }
    else {
      fivestar_comment_insert($comment->cid, $comment->nid, $commnt->uid, $comment->fivestar_rating);
    }
  }
  // Delete.
  else {
    fivestar_comment_delete($comment->cid, $comment->nid, $comment->uid);
  }
}

/**
 * Get a current rating for a comment.
 */
function fivestar_comment_load($cid, $reset = FALSE) {
  static $cids = array();
  if (!isset($cids[$cid]) || $reset) {
    $cids[$cid] = db_result(db_query('SELECT value FROM {fivestar_comment} WHERE cid = %d', $cid));
  }
  return $cids[$cid];
}

/**
 * Update a fivestar comment value.
 */
function fivestar_comment_update($cid, $nid, $uid, $value) {
  db_query('UPDATE {fivestar_comment} SET value = %d WHERE cid = %d', $value, $cid);
  _fivestar_cast_vote('node', $nid, $value, 'vote', $uid);
}

/**
 * Insert a fivestar comment value.
 */
function fivestar_comment_insert($cid, $nid, $uid, $value) {
  db_query('INSERT INTO {fivestar_comment} (cid, value) VALUES (%d, %d)', $cid, $value);
  _fivestar_cast_vote('node', $nid, $value, 'vote', $uid);
}

/**
 * Delete any value for a comment and update their vote.
 */
function fivestar_comment_delete($cid, $nid, $uid) {
  db_query('DELETE FROM {fivestar_comment} WHERE cid = %d', $cid);

  // Find if the user has posted any other comments. If so, use that
  // comment's rating rather than deleting their vote entirely.
  $new_vote = db_result(db_query('SELECT value FROM {fivestar_comment} fc INNER JOIN {comments} c ON fc.cid = c.cid WHERE c.uid = %d AND c.nid = %d ORDER BY timestamp desc', $uid, $nid)); 
  _fivestar_cast_vote('node', $nid, empty($new_vote) ? 0 : $new_vote, 'vote', $uid);
}
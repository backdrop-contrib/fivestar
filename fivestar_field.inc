<?php

/* $Id $ */

/**
 * @file
 * Provides CCK integration for fivestar module
 */

/**
 * Implementation of hook_field_info().
 */
function fivestar_field_info() {
  return array(
    'fivestar' => array('label' => 'Fivestar Rating'),
  );
}

/**
 * Implementation of hook_field_settings().
 */
function fivestar_field_settings($op, $field) {
  switch ($op) {
    case 'form':
      $form['stars'] = array(
        '#type' => 'select',
        '#title' => t('Number of Options'),
        '#options' => drupal_map_assoc(array(1,2,3,4,5,6,7,8,9,10)),
        '#default_value' => $widget['stars'] ? $widget['stars'] : 5,
        '#description' => t('The number of stars or radio buttons to display.'),
      );
      $form['target'] = array(
        '#type' => 'textarea',
        '#title' => t('Node ID'),
        '#default_value' => $field['target'] ? $field['target'] : '',
        '#description' => t(
          'A single node ID on which this field will register the vote. If no ID is specified, the vote will be registered on the created node.'
        ),
      );
      $form['multiple'] = array(); // Multiple not supported
      
      if (user_access('use PHP for fivestar target')) {
        $form['target']['#description'] .= ' ' . t(
          ' Return the target node ID or use plain text. Enter PHP code between <em>&lt;?php ?&gt;</em>. Note that executing incorrect PHP-code can break your Drupal site.'
        );
        
        $form['php'] = array(
          '#type' => 'value',
          '#value' => 1,
        );
      }
      return $form;
    case 'save':
      return array('stars', 'target', 'php');
    case 'database columns':
      $columns = array(
        'target' => array('type' => 'int', 'default' => 'NULL'),
        'rating' => array('type' => 'int', 'default' => 'NULL'),
      );
      return $columns;
  }
}
  
/**
 * Implementation of hook_field().
 */
function fivestar_field($op, &$node, $field, &$items, $teaser, $page) {
  $fieldname = $field['field_name'];
  switch ($op) {
    case 'view':
      foreach ($items as $delta => $item) {
        $items[$delta]['view'] = content_format($field, $item);
      }
      return theme('field', $node, $field, $items, $teaser, $page);
    case 'insert':
    case 'update':
      foreach($items as $delta => $item) {
        if ($field['php']) {
          $items[$delta]['target'] = drupal_eval($item['target']);
        }
        $target_node = node_load($items[$delta]['target']);
        if ($item['rating'] == 0) {
          votingapi_unset_vote('node', $target_node->nid);
        }
        else {
          _fivestar_cast_vote('node', $target_node->nid, $item['rating']);
        }
      }
      break;
  }
}
      
/**
 * Implementation of hook_widget_info().
 */
function fivestar_widget_info() {
  return array(
    'radios' => array(
      'label' => 'Radio Buttons',
      'field types' => array('fivestar'),
    ),
    'stars' => array(
      'label' => 'Stars',
      'field types' => array('fivestar'),
    ),
  );
}

/**
 * Implementation of hook_widget_settings().
 */
function fivestar_widget_settings($op, $widget) {
  switch ($op) {
    case 'form':
      $form = array();
      $form['allow_clear'] = array(
        '#type' => 'select',
        '#title' => t('Allow Clearing'),
        '#options' => array(1 => 'True', 0 => 'False'),
        '#default_value' => $widget['allow_clear'] ? $widget['allow_clear'] : 1,
        '#description' => t("Display a button to clear the user's current vote."),
      );
      return $form;
    case 'save':
      return array('allow_clear');
  }
}

/**
 * Implementation of hook_widget().
 */
function fivestar_widget($op, &$node, $field, &$items) {
  switch ($op) {
    case 'form':
      $form = array();
      
      if ($field['widget']['type'] == 'stars') {
        // Check if the scripts have already been added
        if ($test = strpos(drupal_get_js(), 'jquery.rating.js') === FALSE) {
          drupal_add_js(drupal_get_path('module', 'fivestar') . '/jquery.rating.js');
          drupal_add_js("jQuery(function(){jQuery('div.fivestar-widget').rating();});", 'inline');
        }
      }
      
      $form[$field['field_name']] = array('#tree' => TRUE);
      $form[$field['field_name']][0]['rating'] = array(
        '#type' => 'fivestar',
        '#title' => $field['widget']['label'],
        '#default_value' => $items[0]['rating'],
        '#stars' => $field['stars'] ? $field['stars'] : 5,
        '#allow_clear' => $field['widget']['allow_clear'],
        '#description' => $field['widget']['description'],
        '#weight' => $field['widget']['weight'],
        '#auto_submit' => false,
      );
      $form[$field['field_name']][0]['target'] = array(
        '#type' => 'value',
        '#value' => $field['target'],
      );
      return $form;
  }
}
      
/**
 * Implementation of hook_field_formatter_info().
 */
function fivestar_field_formatter_info() {
  return array(
    'default' => array(
      'label' => 'As Stars',
      'field types' => array('fivestar'),
    ),
    'rating' => array(
      'label' => 'Rating (i.e. 4.2/5)',
      'field types' => array('fivestar'),
    ),
    'percentage' => array(
      'label' => 'Percentage (i.e. 92)',
      'field types' => array('fivestar'),
    ),
  );
}

/**
 * Implementation of hook_field_formatter().
 *
 * The $node argument is necessary so that filter access can be checked on
 * node preview.
 */
function fivestar_field_formatter($field, $item, $formatter, $node) {
  if (!isset($item['rating'])) {
    return '';
  }

  switch ($formatter) {
    case 'rating':
      return round(100/$item['rating'], 1)/$field['stars'];

    case 'percentage':
      return $item['rating'];

    default:
      return theme('fivestar_static', $item['rating'], $field['stars']);
  }
}

<?php

/**
 * @file
 * Provides CCK integration for fivestar module
 */

/**
 * Implementation of hook_field_info().
 */
function fivestar_field_info() {
  return array(
    'fivestar' => array(
      'label' => t('Fivestar Rating'),
      'description' => t('Store a rating for this piece of content.'),
      'default_widget' => 'stars',
      'default_formatter' => 'fivestar_formatter_default',
      'instance_settings' => array(
        'axis' => 'vote',
      ), 
    ),
  );
}

function fivestar_form_field_ui_field_edit_form_alter(&$form, $form_state) {
  $field = $form['#field'];
  if ($field['type'] == 'fivestar') {
    // Multiple values is not supported with Fivestar.
    $form['field']['cardinality']['#access'] = FALSE;
    $form['field']['cardinality']['#value'] = 1;
  }
}

/**
 * Implementation of hook_field_settings_form().
 */
function fivestar_field_settings_form($field, $instance) {

  $form['axis'] = array(
    '#type' => 'select',
    '#required' => TRUE,
    '#title' => 'Voting Tag',
    '#options' => fivestar_get_tags(),
    '#description' => t('The tag this rating will affect. Enter a property on which that this rating will affect, such as <em>quality</em>, <em>satisfaction</em>, <em>overall</em>, etc.'),
    '#default_value' => isset($field['settings']['axis']) ? $field['settings']['axis'] : '',
    '#disabled' => field_has_data($field),
  );

  return $form;
}

function fivestar_field_instance_settings_form($field, $instance) {
  $form = array();

  $form['stars'] = array(
    '#type' => 'select',
    '#title' => $instance['widget']['type'] == 'stars' ? t('Number of Stars') : t('Number of Options'),
    '#options' => drupal_map_assoc(range(1, 10)),
    '#default_value' => isset($instance['settings']['stars']) ? $instance['settings']['stars'] : 5,
  );

 $form['target'] = array(
   '#title' => t('Voting target'),
   '#type' => 'select',
   '#default_value' => isset($instance['settings']['target']) ? $instance['settings']['target'] : '',
   '#options' => fivestar_get_targets($field, $instance),
   '#description' => t('The voting target will make the value of this field cast a vote on another node. Use node reference fields module to create advanced reviews. Use the Parent Node Target when using fivestar with comments. More information available on the <a href="http://drupal.org/handbook/modules/fivestar">Fivestar handbook page</a>.'),
 );

  return $form;
}

/**
 * Implementation of hook_field_presave().
 */
function fivestar_field_presave($obj_type, $object, $field, $instance, $langcode, &$items) {
  $items[0]['stars'] = $instance['settings']['stars'];
}

/**
 * Implementation of hook_field_insert().
 */
function fivestar_field_insert($entity_type, $entity, $field, $instance, $langcode, &$items) {
  _fivestar_field_helper($entity_type, $entity, $field, $instance, $langcode, $items);
}

/**
 * Implementation of hook_field_update().
 */
function fivestar_field_update($entity_type, $entity, $field, $instance, $langcode, &$items) {
  _fivestar_field_helper($entity_type, $entity, $field, $instance, $langcode, $items);
}

/**
 * Implementation of hook_field_delete().
 */
function fivestar_field_delete($entity_type, $entity, $field, $instance, $langcode, &$items) {
  _fivestar_field_helper($entity_type, $entity, $field, $instance, $langcode, $items, 'delete');
}

function _fivestar_field_helper($entity_type, $entity, $field, $instance, $langcode, &$items, $op = '') {
  foreach ($items as $delta => $item) {
    if ((isset($entity->status) && !$entity->status) || $op == 'delete') {
      $rating = 0;
    }
    else {
      $rating = (isset($items[$delta]['rating'])) ? $items[$delta]['rating'] : 0;
    }
    $items[$delta]['target'] = _fivestar_field_target($entity, $field, $instance, $item, $langcode);
    if (!empty($items[$delta]['target'])) {
     _fivestar_cast_vote($items[$delta]['target']['entity_type'], $items[$delta]['target']['entity_id'], $rating, $field['settings']['axis'], $entity->uid, FALSE, TRUE);
     votingapi_recalculate_results($items[$delta]['target']['entity_type'], $items[$delta]['target']['entity_id']);
    }
  }
}

/**
 * Helper function to find the id that should be rated when a field is changed.
 */
function _fivestar_field_target($entity, $field, $instance, $item, $langcode) {
  if (isset($instance['settings']['target'])) {
    $target = fivestar_get_targets($field, $instance, $instance['settings']['target'], $entity, $langcode);
  }
  else {
    // If all else fails, default to voting on the instance the field is attached to.
    list($id, $vid, $bundle) = entity_extract_ids($instance['entity_type'], $entity);
    $target = array(
      'entity_id' => $id,
      'entity_type' => $instance['entity_type'],
    );
  }
  return $target;
}

/**
 * Implementation of hook_field_is_empty().
 */
function fivestar_field_is_empty($item, $field) {
  return empty($item['rating']) || $item['rating'] == '-';
}

/**
 * Implementation of hook_field_widget_info().
 */
function fivestar_field_widget_info() {
  return array(
    'stars' => array(
      'label' => t('Stars'),
      'field types' => array('fivestar'),
      'behaviors' => array('multiple values' => FIELD_BEHAVIOR_NONE),
    ),
    'select' => array(
      'label' => t('Select list'),
      'field types' => array('fivestar'),
      'behaviors' => array('multiple values' => FIELD_BEHAVIOR_NONE),
    ),
  );
}

/**
 * Implementation of hook_widget_settings_form().
 */
function fivestar_widget_settings_form($field, $instance) {
  // @todo: Add some widget settings here if applicable.
}

/**
 * Implementation of hook_field_widget_form().
 */
function fivestar_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  if ($instance['widget']['type'] == 'select') {
    $element = array('#tree' => TRUE);
    $element['rating'] = array(
      '#type' => 'fivestar',
      '#title' => $instance['label'],
      '#default_value' => isset($items[0]['rating']) ? $items[0]['rating'] : NULL,
      '#stars' => isset($instance['settings']['stars']) ? $instance['settings']['stars'] : 5,
      // dunno where these two comes from
      '#allow_clear' => isset($instance['widget']['settings']['allow_clear']) ? $instance['widget']['settings']['allow_clear'] : 0,
      '#description' => isset($instance['widget']['settings']['description']) ? $instance['widget']['settings']['description'] : '',
      '#weight' => $instance['widget']['weight'],
      '#auto_submit' => FALSE,
      '#widget' => $instance['widget']['type'],
      '#required' => $instance['required'],
      '#labels_enable' => FALSE,
    );
    $element['target'] = array(
      '#type' => 'value',
      // again i have no clue where this comes from.
      '#value' => isset($instance['settings']['target']) ? $instance['settings']['target'] : NULL,
    );
    $element['axis'] = array(
      '#type' => 'value',
      '#value' => $field['settings']['axis'],
    );

    // CCK likes to always have a 2D array for form elements.
    $element = array($element);    
  }
  
  if ($instance['widget']['type'] == 'stars') {
    $element = array('#tree' => TRUE);
    $element['rating'] = array(
      '#type' => 'fivestar',
      '#title' => $instance['label'],
      '#default_value' => isset($items[0]['rating']) ? $items[0]['rating'] : NULL,
      '#stars' => isset($instance['settings']['stars']) ? $instance['settings']['stars'] : 5,
      // dunno where these two comes from
      '#allow_clear' => isset($instance['widget']['settings']['allow_clear']) ? $instance['widget']['settings']['allow_clear'] : 0,
      '#description' => isset($instance['widget']['settings']['description']) ? $instance['widget']['settings']['description'] : '',
      '#weight' => $instance['widget']['weight'],
      '#auto_submit' => FALSE,
      '#widget' => $instance['widget']['type'],
      '#required' => $instance['required'],
      '#labels_enable' => FALSE,
    );
    $element['target'] = array(
      '#type' => 'value',
      // again i have no clue where this comes from.
      '#value' => isset($instance['settings']['target']) ? $instance['settings']['target'] : NULL,
    );
    $element['axis'] = array(
      '#type' => 'value',
      '#value' => $field['settings']['axis'],
    );

    // CCK likes to always have a 2D array for form elements.
    $element = array($element);
  }

  return $element;
}

/**
 * Implementation of hook_field_formatter_info().
 */
function fivestar_field_formatter_info() {
  return array(
    'fivestar_formatter_default' => array(
      'label' => t('As Stars'),
      'field types' => array('fivestar'),
    ),
    'fivestar_formatter_rating' => array(
      'label' => t('Rating (i.e. 4.2/5)'),
      'field types' => array('fivestar'),
    ),
    'fivestar_formatter_percentage' => array(
      'label' => t('Percentage (i.e. 92)'),
      'field types' => array('fivestar'),
    ),
    'fivestar_formatter_exposed_stars' => array(
      'label' => t('Exposed Stars'),
      'field types' => array('fivestar'),
      'settings' =>  array(
        'stars' => 5,
        'allow_clear' => FALSE,
        'style' => 'average',
        'feedback_enable' => FALSE,
        'text' => 'average',
      ),
    ),
  );
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function fivestar_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  
  if ($display['type'] != 'fivestar_formatter_exposed_stars') {
    return;
  }

  $element['stars'] = array(
    '#type' => 'select',
    '#title' => t('Number of stars'),
    '#options' => drupal_map_assoc(range(1, 10)),
    '#default_value' => $settings['stars'],
  );
  $element['feedback_enable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable feedback during vote saving and deletion'),
    '#default_value' => $settings['feedback_enable'],
    '#return_value' => 1
  );
  $element['allow_clear'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow users to undo their votes'),
    '#default_value' => $settings['allow_clear'],
    '#return_value' => 1,
  );
  $element['style'] = array(
    '#type' => 'select',
    '#title' => t('Value to display as stars'),
    '#default_value' => $settings['style'],
    '#options' => array(
      'average' => t('Average vote'),
      'user'    => t("User's vote"),
      'smart'   => t("User's vote if available, average otherwise"),
      'dual'    => t("Both user's and average vote"),
    ),
  );
  $element['text'] = array(
    '#type' => 'select',
    '#title' => t('Text to display under the stars'),
    '#default_value' => $settings['text'],
    '#options' => array(
      'none'    => t('No text'),
      'average' => t('Average vote'),
      'user'    => t("User's vote"),
      'smart'   => t("User's vote if available, average otherwise"),
      'dual'    => t("Both user's and average vote"),
    ),
  );

  return $element;
}

/**
 * Implements hook_field_formatter_settings_summary().
 */
function fivestar_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];

  if ($display['type'] != 'fivestar_formatter_exposed_stars') {
    return;
  }

  $summary = t("Stars: @stars, Feedback: @feedback, Undo: @clear, Stars Display: @style, Text Display: @text", array(
    '@stars' => $settings['stars'], 
    '@clear' => ($settings['allow_clear']) ? 'Yes' : 'No', 
    '@feedback' => ($settings['feedback_enable']) ? 'Yes' : 'No',
    '@style' => $settings['style'],
    '@text' => $settings['text']));

  return $summary;
}

/**
 * Implements hook_field_formatter_view().
 */
function fivestar_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();
  $settings = $display['settings'];

  // We are going to built a custom fivestar rating widget if the exposed stars formatter is 
  // selected and the user has the rate content permission. 
  if ($display['type'] == 'fivestar_formatter_exposed_stars' && user_access('rate content')) {
    // The axis/tag is selected from the field instance.
    $tag = (isset($instance['settings']['axis'])) ? $instance['settings']['axis'] : 'vote';
    list($id, $vid, $bundle) = entity_extract_ids($entity_type, $entity);
    $votes = fivestar_get_votes($entity_type, $id, $tag);
    $values['user'] = isset($votes['user']['value']) ? $votes['user']['value'] : 0;
    $values['average'] = isset($votes['average']['value']) ? $votes['average']['value'] : 0;
    $values['count'] = isset($votes['count']['value']) ? $votes['count']['value'] : 0;

    $settings = array(
      'stars' => $settings['stars'],
      'allow_clear' => $settings['allow_clear'],
      'style' => $settings['style'],
      'text' => $settings['text'],
      'content_type' => $entity_type,
      'content_id' => $id,
      'tag' => $tag,
      'autosubmit' => TRUE,
      'title' => FALSE,
      'feedback_enable' => $settings['feedback_enable'],
      'labels_enable' => FALSE,
      'labels' => array(),
    );

    $element[0] = drupal_get_form('fivestar_custom_widget', $values, $settings);

    return $element;
  }

  // If the display type happends to be fivestar_formatter_exposed_stars and the user does not
  // have access to rate content as check above then we'll going to default back to
  // fivestar_formatter_default. !TODO: This should be configurable.
  if ($display['type'] == 'fivestar_formatter_exposed_stars') {
    $display['type'] = 'fivestar_formatter_default';
  }

  $element[0] = array(
    '#theme' => $display['type'],
    '#item' => $items,
    '#instance_settings' => $instance['settings'],
    '#display_settings' => $settings,
  );

  return $element;
}

/**
 * Implements hook_field_prepare_view()
 */
function fivestar_field_prepare_view($entity_type, $entities, $field, $instances, $langcode, &$items) {
  // Fivestar fields need to be visible when the field has been added to an
  // existing instance of a field but also when that instance has not been edited.
  // This will force the render system to output a widget regardless.
  foreach($entities as $key => $entity) {
    if (empty($items[$key])) {
      $items[$key] = array('fivetar');
    }
  }
}

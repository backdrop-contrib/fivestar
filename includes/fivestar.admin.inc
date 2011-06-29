<?php

/**
 * @file
 * Configuration pages for Fivestar module.
 */

/**
 * Callback function for admin/settings/fivestar. Display the settings form.
 */
function fivestar_settings($form, $form_state) {
  module_load_include('inc', 'fivestar', 'includes/fivestar.color');

  $form['widget'] = array(
    '#tree' => FALSE,
    '#type' => 'fieldset',
    '#title' => t('Widget display'),
    '#description' => t('Choose a widget set to be used on your site. Widgets supporting custom colors can be further customized by adjusting the color scheme.'),
    '#weight' => -2,
  );

  $widgets = module_invoke_all('fivestar_widgets');
  $classic_widgets = array();
  $color_widgets = array();
  foreach ($widgets as $path => $name) {
    $directory = dirname($path);
    $matches = file_scan_directory($directory, '/-template\./');
    if (empty($matches)) {
      $classic_widgets[$path] = $name;
    }
    else {
      $color_widgets[$path] = $name;
    }
  }

  // If using a color widget, set the default value to the original path.
  $default_value = variable_get('fivestar_widget', 'default');
  foreach ($color_widgets as $path => $name) {
    if (basename($path) == basename($default_value)) {
      $default_value = $path;
    }
  }

  $form['widget']['fivestar_widget'] = array(
    '#type' => 'radios',
    '#options' => array('default' => t('Default')) + $classic_widgets + $color_widgets,
    '#default_value' => $default_value,
    '#attributes' => array('class' => array('fivestar-widgets')),
  );

  $form['widget']['fivestar_color_widget'] = array(
    '#type' => 'radios',
    '#title' => t('Custom color widgets'),
    '#options' => $color_widgets,
    '#attributes' => array('class' => array('fivestar-widgets')),
  );

  $form['tags'] = array(
    '#tree' => FALSE,
    '#type' => 'fieldset',
    '#title' => t('Voting tags'),
    '#description' => t('Choose the voting tags that will be available for node rating. A tag is simply a category of vote. If you only need to rate one thing per node, leave this as the default "vote".'),
    '#weight' => 3,
   );

  $form['tags']['tags'] = array(
    '#type' => 'textfield',
    '#title' => t('Tags'),
    '#default_value' => variable_get('fivestar_tags', 'vote'),
    '#required' => TRUE,
    '#description' => t('Separate multiple tags with commas.'),
   );

  $form['color'] = fivestar_color_form();
  $form['#validate'][] = 'fivestar_color_form_validate';
  $form['#submit'][] = 'fivestar_color_form_submit';
  $form['#submit'][] ='fivestar_settings_submit';

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#weight' => 45,
  );

  return $form;
}

function fivestar_settings_submit($form, &$form_state) {
  // TODO We could delete all variables for removed tags
  variable_set('fivestar_tags', $form_state['values']['tags']);
  variable_set('fivestar_widget', $form_state['values']['fivestar_widget']);

  // Rebuild the menu in case tags were changed
  menu_rebuild();
}

/**
 * Adds fivestar enaable and position to the node-type configuration form.
 */
function fivestar_node_type_tag_form($form, &$form_state, $node_type, $tag = 'vote') {
  $type_name = $node_type->type;
  $settings = fivestar_get_settings($type_name, $tag);

  $form_state['fivestar_tag'] = $tag;
  $form_state['fivestar_node_type'] = $type_name;

  $form['fivestar'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Fivestar rating for the "@tag" tag', array('@tag' => $tag)),
    '#default_value' => $settings['fivestar'],
    '#return_value' => 1,
    '#weight' => -5,
  );

  $form['fivestar_stars'] = array(
    '#type' => 'select',
    '#title' => t('Number of stars'),
    '#options' => drupal_map_assoc(range(1, 10)),
    '#default_value' => $settings['stars'],
    '#weight' => -4,
  );

  $form['labels'] = array(
    '#type' => 'fieldset',
    '#title' => t('Star Labels'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('These star labels appear as the link title when javascript is enabled as well as the select list options when javascript is disabled.'),
  );

  $form['labels']['fivestar_labels_enable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display labels on mouse over'),
    '#default_value' => $settings['labels_enable'],
    '#return_value' => 1,
    '#weight' => -5,
    '#description' => t('When enabled, the star labels will dynamically appear underneath the stars as the user hovers over each star to provide a more descriptive qualitative meaning for each star value.'),
  );

  // Create the Mouseover text forms for each of the rating options
  // This form depends on the number of stars, and these extra textfields will be hidden with javascript
  for ($n = 0; $n <= 10; $n++) {
    $form['labels']['fivestar_label_'. $n] = array(
      '#type' => 'textfield',
      '#title' => $n > 0 ? t('Star @star label', array('@star' => $n)) : t('Cancel label'),
      '#default_value' => $settings['labels'][$n],
      '#prefix' => '<div id="fivestar-label-'. $n .'" class="fivestar-label">',
      '#suffix' => '</div>',
      '#size' => 30,
    );
  }

  $form['direct'] = array(
    '#type' => 'fieldset',
    '#title' => t('Direct rating widget'),
    '#collapsible' => FALSE,
    '#description' => t('These settings allow you to display a rating widget to your users while they are viewing content of this type. Rating will immediately register a vote for that piece of content.'),
    '#weight' => 2,
  );

  $form['direct']['fivestar_style'] = array(
    '#type' => 'select',
    '#title' => t('Star display style'),
    '#default_value' => $settings['star_display'],
    '#options' => array(
      'average' => t('Display average vote value'),
      'user'    => t('Display user vote value'),
      'smart'   => t('User vote if available, average otherwise'),
      'dual'    => t('Both user and average vote'),
    ),
  );

  $form['direct']['fivestar_text'] = array(
    '#type' => 'select',
    '#title' => t('Text display style'),
    '#default_value' => $settings['text_display'],
    '#options' => array(
      'none'    => t('Display no text beneath stars'),
      'average' => t('Current average in text'),
      'user'    => t('User current vote in text'),
      'smart'   => t('User vote if available, average otherwise'),
      'dual'    => t('Both user and average vote'),
    ),
  );

  $form['direct']['fivestar_title'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show widget title'),
    '#default_value' => $settings['title_display'],
    '#return_value' => 1,
  );

  $form['direct']['fivestar_feedback'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable feedback during vote saving and deletion'),
    '#default_value' => $settings['feedback_enable'],
    '#return_value' => 1
  );

  $form['direct']['fivestar_unvote'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow users to undo their votes'),
    '#default_value' => $settings['unvote_enable'],
    '#return_value' => 1,
  );

  $form['direct']['fivestar_position_teaser'] = array(
    '#type' => 'select',
    '#title' => t('Teaser display'),
    '#default_value' => $settings['position_teaser'],
    '#options' => array(
      'above' => t('Clickable widget above teaser'),
      'below' => t('Clickable widget below teaser'),
      'above_static' => t('Static display above teaser'),
      'below_static' => t('Static display below teaser'),
      'link' => t('Teaser link to full node widget'),
      'hidden' => '<'. t('hidden') .'>',
    ),
  );

  $form['direct']['fivestar_position'] = array(
    '#type' => 'select',
    '#title' => t('Full node display'),
    '#default_value' => $settings['position'],
    '#options' => array(
      'above' => t('Clickable widget above node body'),
      'below' => t('Clickable widget below node body'),
      'above_static' => t('Static display above node body'),
      'below_static' => t('Static display below node body'),
      'hidden' => '<'. t('hidden') .'>',
    ),
  );

  $form['direct']['fivestar_direct_preview'] = array(
    '#type' => 'item',
    '#title' => t('Direct rating widget preview'),
    '#markup' => theme('fivestar_preview', array(
      'style' => $form['direct']['fivestar_style']['#default_value'],
      'text' => $form['direct']['fivestar_text']['#default_value'],
      'stars' => $form['fivestar_stars']['#default_value'],
      'unvote' => $form['direct']['fivestar_unvote']['#default_value'],
      'title' => $form['direct']['fivestar_title']['#default_value'] ? NULL : FALSE,
      'feedback_enable' => $form['direct']['fivestar_feedback']['#default_value'],
      'labels_enable' => $form['labels']['fivestar_labels_enable']['#default_value'],
      'labels' => $settings['labels'],
    )),
  );
  if (!$form['fivestar']['#default_value']) {
    $form['direct']['fivestar_direct_preview']['#markup'] = theme('fivestar_preview_wrapper', array('content' => ''));
  }
  else {
    $form['direct']['fivestar_direct_preview']['#markup'] = theme('fivestar_preview_wrapper', array('content' => $form['direct']['fivestar_direct_preview']['#markup']));
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Additional submit handler for the node type form.
 */
function fivestar_node_type_tag_form_submit($form, &$form_state) {
  $tag = $form_state['fivestar_tag'];
  $type_name = $form_state['fivestar_node_type'];
  $suffix = fivestar_get_suffix($type_name, $tag);

  // Do not save any fivestar variables if fivestar is disabled.
  if (isset($form_state['values']['fivestar']) && $form_state['values']['fivestar'] === 0) {
    foreach ($form_state['values'] as $key => $value) {
      if (strpos($key, 'fivestar') === 0) {
        variable_del($key . '_' . $suffix);
      }
    }
  }
  else {
    if ($form_state['values']['fivestar_labels_enable']) {
      // Merge labels into a single variable.
      $labels = array();
      for ($n = 0; $n <= 10; $n++) {
        $labels[] = $form_state['values']['fivestar_label_' . $n];
        unset($form_state['values']['fivestar_label_' . $n]);
      }

      variable_set('fivestar_labels_' . $suffix, $labels);
    }
    else {
      variable_del('fivestar_labels_' . $suffix);
    }

    // Now save the remaining variables
    foreach ($form_state['values'] as $key => $value) {
      if (strpos($key, 'fivestar') === 0) {
        variable_set($key . '_' . $suffix, $value);
      }
    }
  }

  drupal_set_message('Your changes have been saved.');
}

/**
 * Menu callback function for fivestar/preview/node.
 *
 * Outputs a JSON page containing a Fivestar preview of a node rating widget.
 */
function fivestar_preview() {
$_POST = array (
  'style' => 'average',
  'text' => 'dual',
  'stars' => '5',
  'unvote' => '1',
  'title' => '1',
  'feedback' => '1',
  'labels_enable' => '1',
  'labels' =>
  array (
    0 => 'Cancel rating',
    1 => 'Poor',
    2 => 'Okay',
    3 => 'Good',
    4 => 'Great',
    5 => 'Awesome',
    6 => 'Give it @star/@count',
    7 => 'Give it @star/@count',
    8 => 'Give it @star/@count',
    9 => 'Give it @star/@count',
    10 => 'Give it @star/@count',
  ),
);
  // Perform a few basic security checks.
  $style = check_plain($_POST['style']);
  $text = check_plain($_POST['text']);
  $stars = (int)$_POST['stars'];
  $unvote = (boolean)$_POST['unvote'];
  $title = (boolean)$_POST['title'];
  $feedback_enable = (boolean)$_POST['feedback'];
  $labels_enable = (boolean)$_POST['labels_enable'];
  $labels = (array)$_POST['labels'];
  foreach ($labels as $key => $label) {
    $labels[$key] = filter_xss_admin($label);
  }

  $output = theme('fivestar_preview', array(
    'style' => $style,
    'text' => $text,
    'stars' => $stars,
    'unvote' => $unvote,
    'title' => $title ? NULL : FALSE,
    'feedback' => $feedback_enable,
    'labels_enable' => $labels_enable,
    'labels' => $labels,
  ));
  drupal_add_http_header('Content-Type', 'text/javascript; charset=utf-8');
  print drupal_json_encode(array('status' => TRUE, 'data' => $output));
}

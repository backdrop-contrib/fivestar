<?php
// $Id$

/**
 * @file
 * Configuration pages for Fivestar module.
 */

/**
 * Callback function for admin/settings/fivestar. Display the settings form.
 */
function fivestar_settings() {
  module_load_include('inc', 'fivestar', 'includes/fivestar.color');

  $form = array();

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
    $matches = file_scan_directory($directory, '-template.');
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
    '#attributes' => array('class' => 'fivestar-widgets'),
  );

  $form['widget']['fivestar_color_widget'] = array(
    '#type' => 'radios',
    '#title' => t('Custom color widgets'),
    '#options' => $color_widgets,
    '#attributes' => array('class' => 'fivestar-widgets'),
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

function theme_fivestar_settings($form) {
  drupal_add_css(drupal_get_path('module', 'fivestar') .'/css/fivestar-admin.css', 'module', 'all', FALSE);
  drupal_set_title(t('Fivestar Settings'));

  // Default preview.
  $form['widget']['fivestar_widget']['default']['#description'] = 'Default '. t('Preview') .':<br />'. theme('fivestar_preview_widget', 'default');

  // Preview for each classic widget.
  foreach (element_children($form['widget']['fivestar_widget']) as $widget_key) {
    if ($widget_key != 'default') {
      $form['widget']['fivestar_widget'][$widget_key]['#description'] = $form['widget']['fivestar_widget'][$widget_key]['#title'] .' '. t('Preview') .':<br />'. theme('fivestar_preview_widget', $widget_key);
    }
  }

  // Preview for each color-enabled widget.
  foreach (element_children($form['widget']['fivestar_color_widget']) as $widget_key) {
    $form['widget']['fivestar_color_widget'][$widget_key] = $form['widget']['fivestar_widget'][$widget_key];
    $form['widget']['fivestar_color_widget'][$widget_key]['#description'] = $form['widget']['fivestar_color_widget'][$widget_key]['#title'] .' '. t('Preview') .':<br />'. theme('fivestar_preview_widget', $widget_key);
    unset($form['widget']['fivestar_widget'][$widget_key]);
  }

  // Add the new styles to the page.
  drupal_set_html_head("<style type=\"text/css\" media=\"all\">\n". fivestar_get_inline_css() ."</style>");

  $form['widget']['fivestar_widget']['#attributes']['class'] .= ' clear-block';
  $form['widget']['fivestar_color_widget']['#attributes']['class'] .= ' fivestar-color-widgets clear-block';

  return drupal_render($form);
}

function theme_fivestar_preview_widget($css_file) {
  static $default_css_added = FALSE;

  // Add the default CSS to the page to ensure the defaults take precedence.
  if (!$default_css_added) {
    $css = file_get_contents(drupal_get_path('module', 'fivestar') .'/css/fivestar.css');
    // Prepend the classes with the unique widget div.
    $css = preg_replace('/((div)?\.fivestar-widget)/', 'div.fivestar-widgets $1', $css);
    // Update relative URLs with absolute locations.
    $css = preg_replace('/url\(\.\.\/(.*?)\)/', 'url('. base_path() . drupal_get_path('module', 'fivestar') .'/$1)', $css);
    fivestar_add_inline_css('default', $css);
    $default_css_added = TRUE;
  }

  // Add widget specific CSS to the page.
  $widget_name = str_replace('.css', '', basename($css_file));
  $widget_path = dirname($css_file);
  if ($widget_name != 'default') {
    $css = file_get_contents($css_file);
    // Prepend the classes with the unique widget div.
    $css = preg_replace('/((div)?\.fivestar-widget)/', 'div#fivestar-preview-'. $widget_name .' $1', $css);
    // Update relative URLs with absolute locations.
    $css = preg_replace('/url\((.*?)\)/', 'url('. base_path() . $widget_path .'/$1)', $css);
    fivestar_add_inline_css($widget_name, $css);
  }

  $form = array(
    '#post' => array(),
    '#programmed' => FALSE,
    '#tree' => FALSE,
  );
  $form_state = array();
  $form['vote'] = array(
    '#type' => 'fivestar',
    '#stars' => 5,
    '#auto_submit' => FALSE,
    '#allow_clear' => TRUE,
  );

  $form = form_builder('fivestar_preview', $form, $form_state);

  $output = '<div class="fivestar-star-preview" id="fivestar-preview-'. $widget_name .'">';
  $output .= drupal_render($form);
  $output .= '</div>';

  return $output;
}

/**
 * Menu callback function for fivestar/preview/node.
 *
 * Outputs a JSON page containing a Fivestar preview of a node rating widget.
 */
function fivestar_preview() {
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

  $output = theme('fivestar_preview', $style, $text, $stars, $unvote, $title ? NULL : FALSE, $feedback_enable, $labels_enable, $labels);
  drupal_set_header('Content-Type: text/javascript; charset=utf-8');
  print drupal_to_js(array('status' => TRUE, 'data' => $output));
}

function theme_fivestar_preview($style = NULL, $text = NULL, $stars = NULL, $unvote = NULL, $title = NULL, $feedback_enable = TRUE, $labels_enable = TRUE, $labels = array()) {
  $values = array(
    'average' => 50,
    'user' => 80,
    'count' => 20,
  );
  $settings = array(
    'stars' => $stars,
    'allow_clear' => $unvote,
    'style' => $style,
    'text' => $text,
    'title' => $title,
    'autosubmit' => FALSE,
    'feedback_enable' => $feedback_enable,
    'labels_enable' => $labels_enable,
    'labels' => $labels,
    'tag' => 'vote',
  );

  $form = drupal_get_form('fivestar_custom_widget', $values, $settings);
  // This regex is sadly necessary because having duplicate form_tokens or
  // form_id elements can cause the content type form to choke. Forms inside of
  // forms is also frowned upon, so this removes the wrapping form tag as well.
  $form = str_replace(array('<form', '</form>'), array('<div', '</div>'), $form);
  $form = preg_replace('/( method=".*?")|( action=".*?")|(<input.*?name="(form_token|form_id|destination|form_build_id)".*?\/>)/', '', $form);
  return $form;
}

function theme_fivestar_preview_wrapper($content, $type = 'direct') {
  return '<div class="fivestar-preview fivestar-preview-'. $type .'">'. $content .'</div>';
}

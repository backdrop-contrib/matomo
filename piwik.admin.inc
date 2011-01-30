<?php
// $Id$

/**
 * @file
 * Administrative page callbacks for the piwik module.
 */

/**
 * Implements hook_admin_settings() for configuring the module.
 */
function piwik_admin_settings_form($form_state) {
  $form['account'] = array(
    '#type' => 'fieldset',
    '#title' => t('General settings'),
  );

  $form['account']['piwik_site_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Piwik site ID'),
    '#default_value' => variable_get('piwik_site_id', ''),
    '#size' => 15,
    '#maxlength' => 20,
    '#required' => TRUE,
    '#description' => t('The user account number is unique to the websites domain. Click the <strong>Settings</strong> link in your Piwik account, then the <strong>Sites</strong> tab and enter the <strong>ID</strong> into this field.'),
  );
  $form['account']['piwik_url_http'] = array(
    '#type' => 'textfield',
    '#title' => t('Piwik HTTP URL'),
    '#default_value' => variable_get('piwik_url_http', ''),
    '#size' => 80,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#description' => t('The URL to your Piwik base directory. Example: "http://www.example.com/piwik/".'),
  );
  $form['account']['piwik_url_https'] = array(
    '#type' => 'textfield',
    '#title' => t('Piwik HTTPS URL'),
    '#default_value' => variable_get('piwik_url_https', ''),
    '#size' => 80,
    '#maxlength' => 255,
    '#description' => t('The URL to your Piwik base directory with SSL certificate installed. Required if you track a SSL enabled website. Example: "https://www.example.com/piwik/".'),
  );

  // Visibility settings.
  $form['tracking_title'] = array(
    '#type' => 'item',
    '#title' => t('Tracking scope'),
  );
  $form['tracking'] = array(
    '#type' => 'vertical_tabs',
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'piwik') . '/piwik.admin.js'),
    ),
  );

  // Standard tracking configurations.
  $form['tracking']['user_vis_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Users'),
  );
  $t_permission = array('%permission' => t('opt-in or out of tracking'));
  $form['tracking']['user_vis_settings']['piwik_custom'] = array(
    '#type' => 'radios',
    '#title' => t('Allow users to customize tracking on their account page'),
    '#options' => array(
      t('No customization allowed'),
      t('Tracking on by default, users with %permission permission can opt out', $t_permission),
      t('Tracking off by default, users with %permission permission can opt in', $t_permission)
    ),
    '#default_value' => variable_get('piwik_custom', 0),
  );

  // Render the role overview.
  $form['tracking']['role_vis_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Roles'),
  );

  $role_options = array_map('check_plain', user_roles());
  $form['tracking']['role_vis_settings']['piwik_roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Add tracking for specific roles'),
    '#default_value' => variable_get('piwik_roles', array()),
    '#options' => $role_options,
    '#description' => t('If none are selected, all users will be tracked. If a user has any of the roles checked, that user will be tracked.'),
  );

  // Page specific visibility configurations.
  $php_access = user_access('use PHP for tracking visibility');
  $visibility = variable_get('piwik_visibility', 0);
  $pages = variable_get('piwik_pages', '');

  $form['tracking']['page_vis_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Pages'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  if ($visibility == 2 && !$php_access) {
    $form['tracking']['page_vis_settings'] = array();
    $form['tracking']['page_vis_settings']['visibility'] = array('#type' => 'value', '#value' => 2);
    $form['tracking']['page_vis_settings']['pages'] = array('#type' => 'value', '#value' => $pages);
  }
  else {
    $options = array(
      t('Every page except the listed pages'),
      t('The listed pages only')
    );
    $description = t("Specify pages by using their paths. Enter one path per line. The '*' character is a wildcard. Example paths are %blog for the blog page and %blog-wildcard for every personal blog. %front is the front page.", array('%blog' => 'blog', '%blog-wildcard' => 'blog/*', '%front' => '<front>'));

    if (module_exists('php') && $php_access) {
      $options[] = t('Pages on which this PHP code returns <code>TRUE</code> (experts only)');
      $title = t('Pages or PHP code');
      $description .= ' ' . t('If the PHP option is chosen, enter PHP code between %php. Note that executing incorrect PHP code can break your Drupal site.', array('%php' => '<?php ?>'));
    }
    else {
      $title = t('Pages');
    }
    $form['tracking']['page_vis_settings']['piwik_visibility'] = array(
      '#type' => 'radios',
      '#title' => t('Add tracking to specific pages'),
      '#options' => $options,
      '#default_value' => $visibility,
    );
    $form['tracking']['page_vis_settings']['piwik_pages'] = array(
      '#type' => 'textarea',
      '#title' => $title,
      '#title_display' => 'invisible',
      '#default_value' => $pages,
      '#description' => $description,
      '#wysiwyg' => FALSE,
    );
  }

  // Link specific configurations.
  $form['tracking']['linktracking'] = array(
    '#type' => 'fieldset',
    '#title' => t('Links and downloads'),
  );
  $form['tracking']['linktracking']['piwik_track'] = array(
    '#type' => 'checkbox',
    '#title' => t('Track clicks on outgoing links and downloads (clicks on file links) for the following extensions'),
    '#default_value' => variable_get('piwik_track', 1),
  );
  $form['tracking']['linktracking']['piwik_trackfiles_extensions'] = array(
    '#title' => t('List of download file extensions'),
    '#title_display' => 'invisible',
    '#type' => 'textfield',
    '#default_value' => variable_get('piwik_trackfiles_extensions', PK_TRACKFILES_EXTENSIONS),
    '#description' => t('A file extension list separated by the | character that will be tracked when clicked. Regular expressions are supported. For example: !extensions', array('!extensions' => PK_TRACKFILES_EXTENSIONS)),
    '#maxlength' => 255,
  );

  $form['tracking']['search'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search'),
  );

  $site_search_dependencies = '<div class="admin-dependencies">';
  $site_search_dependencies .= t('Depends on: !dependencies', array('!dependencies' => (module_exists('search') ? t('@module (<span class="admin-enabled">enabled</span>)', array('@module' => 'Search')) : t('@module (<span class="admin-disabled">disabled</span>)', array('@module' => 'Search')))));
  $site_search_dependencies .= '</div>';

  $form['tracking']['search']['piwik_site_search'] = array(
    '#type' => 'checkbox',
    '#title' => t('Track internal search') . ' (EXPERIMENTAL)',
    '#description' => t('If checked, internal search keywords are tracked. You must manually install and configure the Site Search plugin for your sites. For the setting <em>Search URL</em> use a value of <strong>search</strong> and for <em>Search Parameter</em> use a value of <strong>query</strong>. For more information see <a href="@url">SiteSearch plugin for piwik</a>.', array('@url' => 'http://github.com/BeezyT/piwik-sitesearch/wiki')) . $site_search_dependencies,
    '#default_value' => variable_get('piwik_site_search', FALSE),
    '#disabled' => (module_exists('search') ? FALSE : TRUE),
  );

  // Advanced feature configurations.
  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['advanced']['piwik_cache'] = array(
    '#type' => 'checkbox',
    '#title' => t('Locally cache tracking code file'),
    '#description' => t('If checked, the tracking code file is retrieved from your Piwik site and cached locally. It is updated daily to ensure updates to tracking code are reflected in the local copy.'),
    '#default_value' => variable_get('piwik_cache', 0),
  );

  // Allow for tracking of the originating node when viewing translation sets.
  if (module_exists('translation')) {
    $form['advanced']['piwik_translation_set'] = array(
      '#type' => 'checkbox',
      '#title' => t('Track translation sets as one unit'),
      '#description' => t('When a node is part of a translation set, record statistics for the originating node instead. This allows for a translation set to be treated as a single unit.'),
      '#default_value' => variable_get('piwik_translation_set', 0),
    );
  }

  // Code snippet settings.
  $form['advanced']['codesnippet'] = array(
    '#type' => 'fieldset',
    '#title' => t('Custom JavaScript code'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('You can add custom Piwik <a href="@snippets">code snippets</a> here. These will be added to every page that Piwik appears on. <strong>Do not include the &lt;script&gt; tags</strong>, and always end your code with a semicolon (;).', array('@snippets' => 'http://piwik.org/docs/javascript-tracking/'))
  );
  $form['advanced']['codesnippet']['piwik_codesnippet_before'] = array(
    '#type' => 'textarea',
    '#title' => t('Code snippet (before)'),
    '#default_value' => variable_get('piwik_codesnippet_before', ''),
    '#rows' => 5,
    '#wysiwyg' => FALSE,
    '#description' => t('Code in this textarea will be added <strong>before</strong> _paq.push(["trackPageView"]).')
  );
  $form['advanced']['codesnippet']['piwik_codesnippet_after'] = array(
    '#type' => 'textarea',
    '#title' => t('Code snippet (after)'),
    '#default_value' => variable_get('piwik_codesnippet_after', ''),
    '#rows' => 5,
    '#wysiwyg' => FALSE,
    '#description' => t('Code in this textarea will be added <strong>after</strong> _paq.push(["trackPageView"]). This is useful if you\'d like to track a site in two accounts.')
  );

  $form['advanced']['piwik_js_scope'] = array(
    '#type' => 'select',
    '#title' => t('JavaScript scope'),
    '#description' => t("<strong>Warning:</strong> Adding the external JavaScript files to the footer region is recommended for performance reasons."),
    '#options' => array(
      'footer' => t('Footer'),
      'header' => t('Header'),
    ),
    '#default_value' => variable_get('piwik_js_scope', 'footer'),
  );

  return system_settings_form($form);
}

function piwik_admin_settings_form_validate($form, &$form_state) {
  if (!preg_match('/^\d{1,}$/', $form_state['values']['piwik_site_id'])) {
    form_set_error('piwik_site_id', t('A valid Piwik site ID is an integer only.'));
  }

  $url = $form_state['values']['piwik_url_http'] . 'piwik.php';
  $result = drupal_http_request($url);
  if ($result->code != 200) {
    form_set_error('piwik_url_http', t('The validation of "@url" failed with error "@error" (HTTP code @code).', array('@url' => check_url($url), '@error' => $result->error, '@code' => $result->code)));
  }

  if (!empty($form_state['values']['piwik_url_https'])) {
    $url = $form_state['values']['piwik_url_https'] . 'piwik.php';
    $result = drupal_http_request($url);
    if ($result->code != 200) {
      form_set_error('piwik_url_https', t('The validation of "@url" failed with error "@error" (HTTP code @code).', array('@url' => check_url($url), '@error' => $result->error, '@code' => $result->code)));
    }
  }

  // Delete obsolete local cache file.
  if (empty($form_state['values']['piwik_cache']) && $form['advanced']['piwik_cache']['#default_value']) {
    piwik_clear_js_cache();
  }

  // This is for the Newbie's who cannot read a text area description.
  if (preg_match('/(.*)<\/?script(.*)>(.*)/i', $form_state['values']['piwik_codesnippet_before'])) {
    form_set_error('piwik_codesnippet_before', t('Do not include the &lt;script&gt; tags in the javascript code snippets.'));
  }
  if (preg_match('/(.*)<\/?script(.*)>(.*)/i', $form_state['values']['piwik_codesnippet_after'])) {
    form_set_error('piwik_codesnippet_after', t('Do not include the &lt;script&gt; tags in the javascript code snippets.'));
  }

  // Trim some text area values.
  $form_state['values']['piwik_pages'] = trim($form_state['values']['piwik_pages']);
  $form_state['values']['piwik_codesnippet_before'] = trim($form_state['values']['piwik_codesnippet_before']);
  $form_state['values']['piwik_codesnippet_after'] = trim($form_state['values']['piwik_codesnippet_after']);
}

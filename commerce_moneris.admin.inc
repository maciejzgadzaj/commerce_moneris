<?php

/**
 * @file
 * Administration forms for the Commerce Moneris.
 */

/**
 * Menu callback; Displays the administration settings for Moneris Payment.
 */
function commerce_moneris_admin_settings() {
  $form = array();

  $form['commerce_moneris'] = array(
    '#type' => 'fieldset',
    '#title' => t('Moneris Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['commerce_moneris']['commerce_moneris_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => variable_get('commerce_moneris_title', 'Moneris'),
    '#description' => t('Override the default title of the commerce moneris payment method.'),
  );  
  
  $form['commerce_moneris']['commerce_moneris_gateway'] = array(
    '#type' => 'select',
    '#title' => t('Moneris Gateway'),
    '#options' => array(
      'ca' => t('Canada'),
      'us' => t('US'),
    ),
    '#default_value' => variable_get('commerce_moneris_gateway', 'ca'),
  );

  $form['commerce_moneris']['commerce_moneris_production'] = array(
    '#type' => 'radios',
    '#title' => t('Moneris Server'),
    '#options' => array(
      0 => t('Moneris Sandbox'),
      1 => t('Moneris Production'),
    ),
    '#default_value' => variable_get('commerce_moneris_production', 0),
  );

  $form['commerce_moneris']['commerce_moneris_display_response'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display response messages'),
    '#description' => t('Display transaction response messages from Moneris when a payment fails. These messages are useful for debugging but you may want to hide them in production.'),
    '#default_value' => variable_get('commerce_moneris_display_response', 1),
  );

  $form['commerce_moneris']['commerce_moneris_store_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Store ID'),
    '#default_value' => variable_get('commerce_moneris_store_id', ''),
    '#required' => TRUE,
  );

  $form['commerce_moneris']['commerce_moneris_apitoken'] = array(
    '#type' => 'textfield',
    '#title' => t('API Token'),
    '#default_value' => variable_get('commerce_moneris_apitoken', ''),
    '#required' => TRUE,
  );

  $form['commerce_moneris']['ssl'] = array(
    '#type' => 'fieldset',
    '#title' => t('Moneris SSL Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['commerce_moneris']['ssl']['commerce_moneris_ssl'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use SSL'),
    '#default_value' => variable_get('commerce_moneris_ssl', 0),
  );

  $form['commerce_moneris']['ssl']['commerce_moneris_curl_certificate_path'] = array(
    '#type' => 'textfield',
    '#title' => t('cURL certificate'),
    '#description' => t('Path to the cURL certificate cacert.pem. You can download it from: http://curl.haxx.se/docs/caextract.html . This is required in case you are using SSL.'),
    '#default_value' => variable_get('commerce_moneris_curl_certificate_path', ''),
  );

  return system_settings_form($form);
}

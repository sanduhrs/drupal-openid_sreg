<?php
// $Id$

/**
 * @file
 * OpenID SREG page callbacks.
 * 
 * @author
 * Stefan Auditor <stefan.auditor@erdfisch.de>
 */

/**
 * Menu callback for the administration settings page
 * 
 * TODO: Add the nice tabledrag functionality to this page and let
 * the admins drag the items from required <> optional <> disabled.
 * See ?q=admin/build/block
 * TODO: Add mapping to Profile and CCK fields
 */
function openid_sreg_admin_settings_form() {
  $form = array();
  module_load_include('inc', 'openid_sreg');

  //TODO: Only display if necessary
  if (FALSE AND module_exists('openid_provider')) {
    $form['openid_sreg']['response'] = array(
      '#type' => 'fieldset',
      '#title' => t('Provider settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#description' => t('Define which attributes will be returned on an authentication request from a remote OpenID Consumer.'),
    );
    $form['openid_sreg']['response']['openid_sreg_response_required'] = array(
      '#type' => 'textfield',
      '#title' => t('Required attributes'),
      '#default_value' => variable_get('openid_sreg_response_required', 'nickname,email'),
      '#description' => t('List of field names which, if absent from the response, will prevent the Consumer from completing the registration without End User interaction. Possible values are: %values', array('%values' => str_replace('openid.sreg.', '', implode(',', array_keys(openid_sreg_get_attributes()))))),
    );
    $form['openid_sreg']['response']['openid_sreg_response_optional'] = array(
      '#type' => 'textfield',
      '#title' => t('Optional attributes'),
      '#default_value' => variable_get('openid_sreg_response_optional', ''),
      '#description' => t('List of field names that will be used by the Consumer, but whose absence will not prevent the registration from completing. Possible values are: %values', array('%values' => str_replace('openid.sreg.', '', implode(',', array_keys(openid_sreg_get_attributes()))))),
    );
  }
  
  // Only display if necessary
  if (module_exists('openid')) {
    $form['openid_sreg']['request'] = array(
      '#type' => 'fieldset',
      '#title' => t('Consumer settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#description' => t('Define the required and optional attributes to request from a remote OpenID Provider.'),
    );
    $form['openid_sreg']['request']['openid_sreg_request_required'] = array(
      '#type' => 'textfield',
      '#title' => t('Required attributes'),
      '#default_value' => variable_get('openid_sreg_request_required', 'nickname,email'),
      '#description' => t('Comma seperated list of field names which, if absent from the response, will prevent the Consumer from completing the registration without End User interation. Possible values are: %values', array('%values' => str_replace('openid.sreg.', '', implode(',', array_keys(openid_sreg_get_attributes()))))),
    );
    $form['openid_sreg']['request']['openid_sreg_request_optional'] = array(
      '#type' => 'textfield',
      '#title' => t('Optional attributes'),
      '#default_value' => variable_get('openid_sreg_request_optional', ''),
      '#description' => t('Comma seperated list of field names that will be used by the Consumer, but whose absence will not prevent the registration from completing. Possible values are: %values', array('%values' => str_replace('openid.sreg.', '', implode(',', array_keys(openid_sreg_get_attributes()))))),
    );
    $form['openid_sreg']['request']['openid_sreg_request_policy_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Policy URL'),
      '#default_value' => variable_get('openid_sreg_request_policy_url', ''),
      '#size' => 40,
      '#description' => t('A URL which the Consumer provides to give the End User a place to read about the how the profile data will be used.'),
      '#field_prefix' => url(NULL, array('absolute' => TRUE)) . (variable_get('clean_url', 0) ? '' : '?q='),
    );
  }

  return system_settings_form($form);
}

/**
 * Menu callback for the user profile page
 */
function openid_sreg_profile_page($user = NULL) {
  if (!$user) {
    global $user;
  }
  
  drupal_set_title(t('OpenID Simple Registration'));
  return drupal_get_form('openid_sreg_profile_form', $user);
}

/**
 * Profile page form
 */
function openid_sreg_profile_form($form_state, $user) {
  module_load_include('inc', 'openid_sreg');
  
  $attributes = openid_sreg_get_attributes();
  $form_state['storage']['uid'] = $user->uid;
  $form['account'] = array(
    '#type' => 'fieldset',
    '#title' => t('Account data'),
    '#description' => t('These values may be changed on your !account_page.', array('!account_page' => l('account page', 'user/'. $user->uid .'/edit'))),
  );
  $form['personal'] = array(
    '#type' => 'fieldset',
    '#title' => t('Personal data'),
    //'#description' => t('These settings may be changed on your !profile_page.', array('!profile_page' => l('profile page', 'user/'. $user->uid .'/edit'))),
  );
  
  foreach ($attributes as $element => $values) {
    $attribute = str_replace('openid.sreg.', '', $element);
    switch ($attribute) {
      case 'nickname':
        $form['account'][$attribute] = array(
          '#type' => 'textfield',
          '#title' => $values['label'],
          '#default_value' => openid_sreg_user_get_attributes($user, $attribute),
          '#description' => t('!description, e.g. !example.', array('!description' => $attributes[$element]['description'], '!example' => $attributes[$element]['example'])),
          '#attributes' => array('disabled' => 'disabled'),
        );
      case 'email':
        $form['account'][$attribute] = array(
          '#type' => 'textfield',
          '#title' => $values['label'],
          '#default_value' => openid_sreg_user_get_attributes($user, $attribute),
          '#description' => t('!description, e.g. !example.', array('!description' => $attributes[$element]['description'], '!example' => $attributes[$element]['example'])),
          '#attributes' => array('disabled' => 'disabled'),
        );
        break;
      case 'dob':
        $form['personal'][$attribute] = array(
          '#type' => 'date',
          '#title' => $values['label'],
          '#default_value' => openid_sreg_user_get_attributes($user, $attribute),
          '#description' => t('!description, e.g. !example.', array('!description' => $attributes[$element]['description'], '!example' => $attributes[$element]['example'])),
          '#attributes' => user_access('edit own sreg page') ? NULL : array('disabled' => 'disabled'),
        );
        break;
      case 'gender':
        $form['personal'][$attribute] = array(
          '#type' => 'select',
          '#title' => $values['label'],
          '#default_value' => openid_sreg_user_get_attributes($user, $attribute),
          '#description' => t('!description, e.g. !example.', array('!description' => $attributes[$element]['description'], '!example' => $attributes[$element]['example'])),
          '#options' => array('F' => t('female'), 'M' => t('male')),
          '#attributes' => user_access('edit own sreg page') ? NULL : array('disabled' => 'disabled'),
        );
        break;
      case 'timezone':
        $form['personal'][$attribute] = array(
          '#type' => 'select',
          '#title' => $values['label'],
          '#default_value' => openid_sreg_user_get_attributes($user, $attribute),
          '#description' => t('!description, e.g. !example.', array('!description' => $attributes[$element]['description'], '!example' => $attributes[$element]['example'])),
          '#options' => drupal_map_assoc(openid_sreg_get_timezones()),
          '#attributes' => user_access('edit own sreg page') ? NULL : array('disabled' => 'disabled'),
        );
        break;
      case 'country':
        $form['personal'][$attribute] = array(
          '#type' => 'select',
          '#title' => $values['label'],
          '#default_value' => openid_sreg_user_get_attributes($user, $attribute),
          '#description' => t('!description, e.g. !example.', array('!description' => $attributes[$element]['description'], '!example' => $attributes[$element]['example'])),
          '#options' => openid_sreg_get_country_codes(),
          '#attributes' => user_access('edit own sreg page') ? NULL : array('disabled' => 'disabled'),
        );
        break;
      case 'language':
        $form['personal'][$attribute] = array(
          '#type' => 'select',
          '#title' => $values['label'],
          '#default_value' => openid_sreg_user_get_attributes($user, $attribute),
          '#description' => t('!description, e.g. !example.', array('!description' => $attributes[$element]['description'], '!example' => $attributes[$element]['example'])),
          '#options' => openid_sreg_locale_prepare_predefined_list(),
          '#attributes' => user_access('edit own sreg page') ? NULL : array('disabled' => 'disabled'),
        );
        break;
      default:
        $form['personal'][$attribute] = array(
          '#type' => 'textfield',
          '#title' => $values['label'],
          '#default_value' => openid_sreg_user_get_attributes($user, $attribute),
          '#description' => t('!description, e.g. !example.', array('!description' => $attributes[$element]['description'], '!example' => $attributes[$element]['example'])),
          '#attributes' => user_access('edit own sreg page') ? NULL : array('disabled' => 'disabled'),
        );
        break;
    }
  }
  if (user_access('edit own sreg page')) {
    $form['buttons']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save configuration'),
    );
    $form['buttons']['reset'] = array(
      '#type' => 'submit',
      '#value' => t('Reset to defaults'),
    );
  }
  return $form;
}

/**
 * Profile page form submit handler
 */
function openid_sreg_profile_form_submit($form, &$form_state) {
  module_load_include('inc', 'openid_sreg');
  $uid = $form_state['storage']['uid'];
  
  if (is_array($form_state['values'])) {
    db_query("DELETE FROM {openid_sreg} WHERE uid=%d", $uid);
  }
  if ($form_state['values']['op'] == t('Save configuration')) {
    openid_sreg_save_attributes($uid, $form_state['values']);
  }
}
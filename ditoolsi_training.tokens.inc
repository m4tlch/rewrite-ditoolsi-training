<?php

/**
 * Implements hook_token_info().
 */
function ditoolsi_training_token_info() {
  $info = array();

  // Define a new token type.
  $info['types']['notification_options'] = array(
    'name' => t('Notification tokens'),
    'description' => t('These tokens are for using in processing notification events'),
    'needs-data' => 'notification_options'
  );

  // Define any new tokens.
  $info['tokens']['notification_options']['notification_mail'] = array(
    'name' => t('Notification mail'),
    'description' => t('Recipient mail for notification (used only for notification event)'),
  );

  $info['tokens']['notification_options']['notification_mail_body'] = array(
    'name' => t('Notification mail body'),
    'description' => t('Mail body for notification (used only for notification event)'),
  );

  return $info;
}

/**
 * Implements hook_tokens().
 */
function ditoolsi_training_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();

  if ($type == 'notification_options') {
    // Loop through the available tokens.
    foreach ($tokens as $name => $original) {
      // Find our custom tokens by name.
      switch ($name) {
        case 'notification_mail_body':
          // Work out the value of our token.
          $course = $data['notification_options']['course'];
          $term   = $data['notification_options']['term'];

          $query = db_select('node', 'n');
          $query->innerJoin('field_data_field_notification_type', 'nt', 'nt.entity_id = n.nid');
          $query->innerJoin('field_data_field_notification_course', 'nc', 'nc.entity_id = n.nid');
          $query->innerJoin('field_data_body', 'nb', 'nb.entity_id = n.nid');
          $query->condition('nt.field_notification_type_tid', $term->tid);
          $query->condition('nc.field_notification_course_target_id', $course->nid);
          $query->fields('nb', array('body_value'));
          $result = $query->execute()->fetchField();
          $value = $result;
          $replacements[$original] = $value;
          break;

        case 'notification_mail':
          $account = $data['notification_options']['account'];
          $value = $account->mail;
          $replacements[$original] = $value;
          break;
      }
    }
  }

  return $replacements;
}
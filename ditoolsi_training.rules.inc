<?php

/**
* Implementation of hook_rules_data_info().
*
*/
function ditoolsi_training_rules_data_info() {
  return array(
    'notification_options' => array(
      'label' => t('Notification options (array of options for notification)'),
      'group' => t('Ditoolsi training'),
    ),
  );
}

/**
* Implementation of hook_rules_event_info().
*
*/
function ditoolsi_training_rules_event_info() {
  $events = array(
    'ditoolsi_training_pupils_notification' => array(
      'label' => t('Notification for pupils'),
      'group' => t('Dittolsi training'),
      'variables' => array(
        'notification_options' => array(
          'type' => 'notification_options',
          'label' => t('Notification options'),
        ),
      ),
    ),
  );

  return $events;
}

/**
* Implementation of hook_rules_condition_info().
*
*/
function ditoolsi_training_rules_condition_info() {
  $conditions = array(
    'ditoolsi_training_condition_notification_exists' => array(
      'label' => t('Notification exists for course'),
      'group' => t('Dittolsi training'),
      'parameter' => array(
        'notification_options' => array(
          'type' => 'notification_options',
          'label' => t('Notification options to check'),
          'restriction' => 'direct',
        ),
      ),
    ),
  );

  return $conditions;
}

function ditoolsi_training_condition_notification_exists($notification_options) {
  $course = $notification_options['course'];
  $term   = $notification_options['term'];

  $query = db_select('node', 'n');
  $query->innerJoin('field_data_field_notification_type', 'nt', 'nt.entity_id = n.nid');
  $query->innerJoin('field_data_field_notification_course', 'nc', 'nc.entity_id = n.nid');
  $query->condition('nt.field_notification_type_tid', $term->tid);
  $query->condition('nc.field_notification_course_target_id', $course->nid);
  $query->condition('n.status', NODE_PUBLISHED);
  $query->fields('n', array('nid'));
  $result = $query->execute()->fetchField();

  return !empty($result);
}
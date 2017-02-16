<?php 

/**
 * Implements hook_views_data().
 */
function ditoolsi_training_views_data() {
  $data = array();

  $data['ditoolsi_training_pupils_list']['table']['group'] = t('DiToolsi: Training');
  $data['ditoolsi_training_pupils_list']['table']['base']  = array(
    'field' => 'id',
    'title' => t('DiToolsi: Training'),
  );

  $data['ditoolsi_training_pupils_list']['id'] = array(
    'title' => t('Id'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  $data['ditoolsi_training_pupils_list']['course_id'] = array(
    'title' => t('Course ID'),
    'field' => array(
      'handler' => 'views_handler_field_node',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  $data['ditoolsi_training_pupils_list']['user_id'] = array(
    'title' => t('User id'),
    'field' => array(
      'handler' => 'views_handler_field_user',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'relationship' => array(
      'base' => 'users',
      'base field' => 'uid',
      'field' => 'user_id',
      'handler' => 'views_handler_relationship',
      'label' => t('User'),
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_user_uid',
      'numeric' => TRUE,
      'validate type' => 'uid',
    ),
  );

  $data['ditoolsi_training_pupils_list']['current_lesson'] = array(
    'title' => t('Lesson ID'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  $data['ditoolsi_training_pupils_list']['created'] = array(
    'title' => t('Created'),
    'field' => array(
      'handler' => 'views_handler_field_date',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  $data['ditoolsi_training_pupils_list']['status'] = array(
    'title' => t('Status'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  $data['ditoolsi_training_pupils_list']['table']['join'] = array(
    'users' => array(
      'left_field' => 'uid',
      'field' => 'user_id',
    ),
  );

  return $data;
}
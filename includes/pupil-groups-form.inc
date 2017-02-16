<?php

function mgc_pupil_groups($form, &$form_state) {

  global $user;

  $table_users = array();
  $number=1;
  $groups = get_groups_by_teacher($user);
  foreach ($groups as $group) {
    $table_users[$group->tid] = array(
      'number'     => $number++,
      'group_name' => $group->name,
    );
  }

  $header  = array(
    'number'     => t('№'),
    'group_name' => t('Name'),
  );

  $form['table'] = array(
    '#type'    => 'tableselect',
    '#header'  => $header,
    '#options' => $table_users,
    '#empty'   => 'Вы пока не создали ни одной группы',
  );

  $form['delete_groups'] = array(
    '#type'   => 'submit',
    '#value'  => t('Delete'),
    '#ajax'   => array(
      'callback' => 'mgc_groups_form_refresh',
      'wrapper'  => 'pupil-groups-form',
    ),
    '#submit' => array('mgc_group_delete_submit'),
  );

  $form['add_group'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array( 'add_group_wrap' ),
    ),
  );

  $form['add_group']['group_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Добавить новую группу',
    '#attributes' => array(
      'placeholder' => 'Название группы'
    ),
    '#size' => 60,
    '#maxlength' => 128,
  );

  $form['add_group']['add_button'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
    '#ajax' => array(
      'callback' => 'mgc_groups_form_refresh',
      'wrapper'  => 'pupil-groups-form'
    ),
    '#submit' => array('mgc_group_add_submit')
  );


  $form['#attributes'] = array(
    'class' => array( 'pupil-groups-form' ),
  );

  $form['#id'] = 'pupil-groups-form';


  return $form;

}

function mgc_groups_form_refresh($form, &$form_state ) {
  return $form;
}


function mgc_group_add_submit($form, &$form_state) {

  if(!empty($form_state['values']['group_name'])){

    global $user;
    $vocabulary = taxonomy_vocabulary_machine_name_load('user_group');

    $term = new stdClass();
    $term->name = $form_state['values']['group_name'];
    $term->vid = $vocabulary->vid;
    $term->parent = 0;
    $term->field_teacher_owner['und'][0]['value'] = $user->uid;
    taxonomy_term_save($term);

  }

  $form_state['rebuild'] = TRUE;

}


function mgc_group_delete_submit($form, &$form_state) {

  $group_ids = array_filter($form_state['values']['table']);
  $group_ids = array_values($group_ids);

  if (!empty($group_ids)) {

    $uids = get_user_id_by_group($group_ids);
    $users = user_load_multiple($uids);
    foreach ($users as $user) {
      $user->field_user_group = NULL;
      user_save($user);
    }
    foreach ($group_ids as $gid) {
      taxonomy_term_delete($gid);
    }

  }

  $form_state['rebuild'] = TRUE;

}

function get_user_id_by_group($group_ids = array()){

  $user_ids = db_select('field_data_field_user_group', 'g')
    ->fields('g', array('entity_id'))
    ->condition('g.bundle', 'user')
    ->condition('g.field_user_group_tid', $group_ids, 'IN')
    ->execute()
    ->fetchCol();

  return $user_ids;

}
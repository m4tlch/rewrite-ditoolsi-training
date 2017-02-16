<?php
/**
 * [ditoolsi_training_page description]
 * @return [type] [description]
 */
function ditoolsi_training_page() {
  $account = ditoolsi_training_get_teacher();
  $query   = ditoolsi_training_get_cources_query(NULL, TRUE);

  if (isset($account->data['_order_courses']) && is_array($account->data['_order_courses'])) {
    $query->orderBy('field(course.nid, ' . implode(', ', $account->data['_order_courses']) . ')');
  }
  else {
    $query->orderBy('course.created', 'desc');
  }

  $result = $query->execute()->fetchAllKeyed(0, 1);
  $nids   = array_keys($result);

  if (count($nids) == 1 && !ditoolsi_training_access('add course')) {
    drupal_goto('course/' . $nids[0]);
  }

  $variable = variable_get('training_first_session', array());
  drupal_add_js(array(
    'dtTraining' => array(
      'firstSessionVideo' => isset($variable['video']) ? $variable['video'] : '',
    ),
  ), 'setting');

  $nodes = node_load_multiple($nids);
  $items = array();
  $form  = drupal_get_form('ditoolsi_training_filter_courses_form');

  foreach ($nodes as $node) {
    $item = new stdClass();

    $item->wrapper = entity_metadata_wrapper('node', $node);
    $item->node    = $node;
    $item->picture = $item->wrapper->field_course_image->value();
    $item->title   = $node->title;

    $description       = $item->wrapper->body->value();
    $item->description = check_markup($description['value'], $description['format']);
    $item->type        = $item->wrapper->field_course_type->value();
    $item->pupils      = $result[ $node->nid ];
    $item->blocked     = ditoolsi_training_is_user_blocked($node);

    $items[] = $item;
  }

  $page = theme('ditoolsi_training_courses_list', array(
    'items' => $items,
    'form'  => $form,
  ));

  drupal_add_library('system', 'ui.sortable');
  drupal_add_css(drupal_get_path('theme', 'bootstrap_subtheme') . '/css/courses-list.css');

  return $page;
}

function ditoolsi_training_course_page($course) {
  global $user;

  $variable = variable_get('training_first_session', array());
  drupal_add_js(array(
    'dtTraining' => array(
      'firstSessionVideo' => isset($variable['video']) ? $variable['video'] : '',
    ),
  ), 'setting');

  drupal_add_js('https://www.youtube.com/iframe_api', 'external');

  drupal_add_css(drupal_get_path('theme', 'bootstrap_subtheme') . '/css/course.css');
  drupal_add_js(drupal_get_path('theme',
      'bootstrap_subtheme') . '/js/jquery/jquery.readmore.min.js');
  drupal_add_js(drupal_get_path('module', 'ditoolsi_training') . '/js/course.js');


  $wrapper = entity_metadata_wrapper('node', $course);
  $body    = $wrapper->body->value();

  $pupil = db_select('ditoolsi_training_pupils_list', 'pl')
    ->fields('pl', array('current_lesson', 'status', 'started'))
    ->condition('pl.user_id', $user->uid)
    ->condition('pl.course_id', $course->nid)
    ->execute()
    ->fetchObject();

  if ($pupil && !$pupil->status) {
    drupal_set_title($course->title);
    if (!empty($course->field_course_main_courator[ LANGUAGE_NONE ])) {
      $uid          = $course->field_course_main_courator[ LANGUAGE_NONE ][0]['target_id'];
      $name         = ditoolsi_profile_name($uid);
      $message_link = l($name, 'im/user-load/' . $uid, array(
        'attributes' => array(
          'target' => '_blank',
        ),
      ));
    }
    else {
      $message_link = l('Список контактов', 'im/list-contacts', array(
        'attributes' => array(
          'target' => '_blank',
        ),
      ));
    }

    return '<p>Этот курс для Вас заблокирован. За информацией Вы можете обратиться к куратору ' . $message_link . '</p>';
  }

  // if ($pupil && empty($pupil->started)) {
  //   db_update('ditoolsi_training_pupils_list')
  //     ->fields(array(
  //       'started' => REQUEST_TIME,
  //     ))
  //     ->condition('user_id', $user->uid)
  //     ->condition('course_id', $course->nid)
  //     ->execute();
  // }

  $name              = ditoolsi_profile_name($course->uid);
  $current_lesson    = ditoolsi_training_get_current_lesson($course);
  $lessons           = ditoolsi_training_get_lessons($course, $current_lesson);
  $has_actual_lesson = FALSE;
  $in_review         = TRUE;

  foreach ($lessons as $lesson) {
    if ($pupil && empty($lesson->progress)) {
      dt_training_update_start_time_progress($lesson, $user);
    }

    if ($pupil && $lesson->field_lesson_number[ LANGUAGE_NONE ][0]['value'] == $pupil->current_lesson) {
      $query = new EntityFieldQuery();
      $query->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'lesson_progress')
        ->propertyCondition('uid', $user->uid)
        ->fieldCondition('field_progress_lesson_id', 'target_id', $lesson->nid);

      $result = $query->execute();

      if (isset($result['node'])) {
        $nid        = key($result['node']);
        $progress   = node_load($nid);
        $pl_wrapper = entity_metadata_wrapper('node', $progress);

        if (isset($progress->field_progress_status[ LANGUAGE_NONE ][0]['value'])) {
          $status = $progress->field_progress_status[ LANGUAGE_NONE ][0]['value'];
        }
        else {
          $status = -1;
        }

        if ($status == DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW) {
          drupal_set_message(t('You can send a lesson for review'));
          $has_actual_lesson = TRUE;
        }
        elseif ($status == DITOOLSI_TRAINING_PROGRESS_STATUS_NEW && $pupil->started) {
          $number = $lesson->field_lesson_number[ LANGUAGE_NONE ][0]['value'];
          drupal_set_message(t('You accessible lesson #@number, review the material and go through the homework. !link',
            array(
              '@number' => $number,
              '!link'   => '<br>' . l(t('Go to a lesson'),
                  'course/' . $course->nid . '/' . $lesson->nid),
            )));
          user_cookie_save(array('next_lesson' => $lesson->nid));
          $has_actual_lesson = TRUE;
        }
        elseif ($status == DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST) {
          $task_number = -1;

          foreach ($pl_wrapper->field_progress_tasks->value() as $progress_task) {
            if ($progress_status = field_get_items('field_collection_item', $progress_task,
              'field_progress_status')
            ) {
              if (in_array($progress_status[0]['value'], array(
                DITOOLSI_TRAINING_PROGRESS_STATUS_NEW,
                DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST,
                DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW,
              ))) {
                $task_progress = node_load($progress_task->field_progress_task_progress[ LANGUAGE_NONE ][0]['target_id']);
                $task1         = node_load($task_progress->field_progress_task[ LANGUAGE_NONE ][0]['target_id']);
                $task_number   = 0;
                foreach ($lesson->tasks as $task) {
                  if ($task->nid == $task1->nid) {
                    $valid_task_number = TRUE;
                    break;
                  }
                  $task_number++;
                }
                break;
              }
            }
          }

          if (empty($valid_task_number)) {
            $task_number = -1;
          }

          drupal_set_message(t('Teacher asks to complete your homework. !link', array(
            '!link' => '<br>' . l(t('Go to a task'), 'course/' . $course->nid . '/' . $lesson->nid,
                array(
                  'fragment' => 'to-task-' . $task_number,
                )),
          )));
          user_cookie_save(array('next_lesson' => $lesson->nid));
          $has_actual_lesson = TRUE;
        }
        break;
      }
      elseif ($pupil->started) {
        $number = $lesson->field_lesson_number[ LANGUAGE_NONE ][0]['value'];
        drupal_set_message(t('You accessible lesson #@number, review the material and go through the homework. !link',
          array(
            '@number' => $number,
            '!link'   => '<br>' . l(t('Go to a lesson'),
                'course/' . $course->nid . '/' . $lesson->nid),
          )));
        user_cookie_save(array('next_lesson' => $lesson->nid));
        $has_actual_lesson = TRUE;
      }
      break;
    }
  }

  if (!$has_actual_lesson) {
    $query = db_select('ditoolsi_training_pupils_list', 'pl')
      ->fields('pl', array('current_lesson', 'course_id', 'user_id'))
      ->fields('lc', array('entity_id'))
      ->fields('ps', array('field_progress_status_value'))
      ->condition('pl.user_id', $user->uid)
      ->condition('pl.course_id', $course->nid, '<>');

    $or = db_or();
    $or->condition('ps.field_progress_status_value', array(
      DITOOLSI_TRAINING_PROGRESS_STATUS_NEW,
      DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST,
      DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW,
    ), 'IN')->isNull('ps.field_progress_status_value');

    $query->condition($or);
    $query->join('node', 'n', 'n.nid = pl.course_id');
    $query->join('field_data_field_lesson_course', 'lc',
      'lc.field_lesson_course_target_id = n.nid');
    $query->join('field_data_field_lesson_number', 'ln',
      'ln.entity_id = lc.entity_id AND ln.field_lesson_number_value = pl.current_lesson');
    $query->leftJoin('field_data_field_progress_lesson_id', 'pli',
      'pli.field_progress_lesson_id_target_id = lc.entity_id');
    $query->leftJoin('field_data_field_progress_status', 'ps', 'ps.entity_id = pli.entity_id');

    $object = $query->orderBy('ln.field_lesson_number_value', 'desc')
      ->distinct()
      ->execute()
      ->fetchObject();

    if ($object) {
      if (isset($status) && $status == DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW) {
        drupal_set_message(t('Your assignment for this course now checked. You can !link in other course',
          array(
            '!link' => l(t('begin the lessons'),
              'course/' . $object->course_id . '/' . $object->entity_id),
          )));
        unset($in_review);
      }
      elseif ($pupil) {
        if ($pupil->started) {
          drupal_set_message(t('Not available to carry out tasks for this course. You can !link in other course',
            array(
              '!link' => l(t('begin the lessons'),
                'course/' . $object->course_id . '/' . $object->entity_id),
            )));
        }
        else {
          $url = url("course/{$course->nid}/start-learn2");
          drupal_set_message("Для начала обучения нажмите <a href='{$url}'>начать обучение</a>");
        }
        // drupal_set_message('Нет доступных к выполнению заданий на этом курсе. Вы можете <a href="' . url('course/' . $object->course_id . '/' . $object->entity_id) . '">приступить к выполнению уроков</a> на другом курсе');
        unset($in_review);
      }
    }
    elseif ($pupil && !$pupil->started) {
      $url = url("course/{$course->nid}/start-learn2");
      drupal_set_message("Для начала обучения нажмите <a href='{$url}'>начать обучение</a>");
    }
  }

  if (isset($status) && isset($in_review) && $status == DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW) {
    drupal_set_message(t('Your task now at checkout. Wait for the verification'));
  }

  $html = theme('ditoolsi_training_course_page', array(
    'node'           => $course,
    'title'          => $course->title,
    'description'    => isset($body['value']) ? check_markup($body['value'], $body['format']) : '',
    'lessons'        => $lessons,
    'wrapper'        => $wrapper,
    'current_lesson' => $current_lesson,
    'pupil'          => $pupil,
    'name'           => $name,
  ));

  return $html;
}

function ditoolsi_training_lesson_page($course, $lesson) {
  global $user;

  $pupil = db_select('ditoolsi_training_pupils_list', 'pl')
    ->fields('pl', array('current_lesson', 'status', 'started'))
    ->condition('pl.user_id', $user->uid)
    ->condition('pl.course_id', $course->nid)
    ->execute()
    ->fetchObject();

  if ($pupil && !$pupil->status) {
    drupal_set_title($lesson->title);
    if (!empty($course->field_course_main_courator[ LANGUAGE_NONE ])) {
      $uid          = $course->field_course_main_courator[ LANGUAGE_NONE ][0]['target_id'];
      $name         = ditoolsi_profile_name($uid);
      $message_link = l($name, 'im/user-load/' . $uid, array(
        'attributes' => array(
          'target' => '_blank',
        ),
      ));
    }
    else {
      $message_link = l('Список контактов', 'im/list-contacts', array(
        'attributes' => array(
          'target' => '_blank',
        ),
      ));
    }

    return '<p>Этот урок для Вас заблокирован. За информацией Вы можете обратиться к куратору ' . $message_link . '</p>';
  }


  drupal_add_js('https://www.youtube.com/iframe_api', 'external');

  drupal_add_css(drupal_get_path('theme', 'bootstrap_subtheme') . '/css/lesson.css');
  drupal_add_css(drupal_get_path('theme',
      'bootstrap_subtheme') . '/css/jquery/jquery.sliderTabs.min.css');
  drupal_add_js(drupal_get_path('theme', 'bootstrap_subtheme') . '/js/jquery/jquery.sliderTabs.js');
  drupal_add_js(drupal_get_path('module', 'ditoolsi_training') . '/js/lesson.js');

  $wrapper = entity_metadata_wrapper('node', $lesson);

  module_load_include('inc', 'ditoolsi_training', 'includes/form');

  $nids = array_keys($lesson->tasks);

  if ($nids) {
    $query = db_select('node', 'n');
    $query->join('field_data_field_progress_lesson_id', 'lesson_id', 'lesson_id.entity_id = n.nid');
    $query->join('field_data_field_progress_tasks', 'progress_tasks',
      'progress_tasks.entity_id = n.nid');
    $query->join('field_data_field_progress_task_progress', 'task_progress',
      'task_progress.entity_id = progress_tasks.field_progress_tasks_value');
    $query->join('node', 'node_progress_task',
      'node_progress_task.nid = task_progress.field_progress_task_progress_target_id');
    $query->join('field_data_field_progress_task', 'progress_task',
      'progress_task.entity_id = node_progress_task.nid');
    $query->join('node', 'node_task',
      'node_task.nid = progress_task.field_progress_task_target_id');
    $query->join('field_data_field_progress_time_lock', 'time_lock',
      'time_lock.entity_id = node_progress_task.nid');
    $query->join('field_data_field_autotest_freeze_time', 'freeze_time',
      'freeze_time.entity_id = node_task.nid');

    $query->fields('node_progress_task', array('nid'))
      ->condition('n.type', 'lesson_progress')
      ->condition('node_progress_task.type', 'auto_test_progress')
      ->condition('node_task.type', 'lesson_auto_test')
      ->condition('node_task.nid', $nids)
      ->condition('n.uid', $user->uid)
      ->condition('node_progress_task.uid', $user->uid);

    $query->addField('time_lock', 'field_progress_time_lock_value', 'time_lock');
    $query->addField('freeze_time', 'field_autotest_freeze_time_value', 'freeze_time');

    $items = $query->execute()->fetchAll();
  }
  else {
    $items = array();
  }

  $nids = array();

  foreach ($items as $item) {
    $freeze_time = $item->freeze_time * 60;

    if ((REQUEST_TIME - $freeze_time) > $item->time_lock && $item->time_lock) {
      $nids[] = $item->nid;
    }
  }

  $nodes = node_load_multiple($nids);

  foreach ($nodes as $nid => $node) {
    $node->field_progress_time_lock[ LANGUAGE_NONE ][0]['value']      = 0;
    $node->field_progress_blocked[ LANGUAGE_NONE ][0]['value']        = 0;
    $node->field_progress_start_autotest[ LANGUAGE_NONE ][0]['value'] = 0;
    node_save($node);
  }

  $current_lesson = ditoolsi_training_get_current_lesson($course);
  $lessons        = ditoolsi_training_get_lessons($course, $current_lesson);
  $flag           = flag_get_user_flags('node', $lesson->nid, $user->uid, $sid = NULL);

  if (!isset($flag['mark_first_time_student'])) {
    $flag_pupil = flag('flag', 'mark_first_time_student', $lesson->nid, $user, TRUE);
  }


  $current_lesson    = ditoolsi_training_get_current_lesson($course);
  $lessons           = ditoolsi_training_get_lessons($course, $current_lesson);
  $has_actual_lesson = FALSE;
  $in_review         = TRUE;
  $stop              = 'Stop';
  if (empty($lesson->progress)) {
    dt_training_update_start_time_progress($lesson, $user);
  }

  foreach ($lessons as $lesson_item) {
    if ($pupil && $lesson_item->field_lesson_number[ LANGUAGE_NONE ][0]['value'] == $current_lesson) {
      $query = new EntityFieldQuery();
      $query->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'lesson_progress')
        ->propertyCondition('uid', $user->uid)
        ->fieldCondition('field_progress_lesson_id', 'target_id', $lesson_item->nid);

      $result = $query->execute();
      $stop   = 'Stop';
      if (isset($result['node'])) {
        $nid         = key($result['node']);
        $progress    = node_load($nid);
        $pl_wrapper  = entity_metadata_wrapper('node', $progress);
        $task_number = -1;
        $stop        = 'Stop';
        foreach ($pl_wrapper->field_progress_tasks->value() as $progress_task) {
          $stop = 'Stop';
          if ($progress_status = field_get_items('field_collection_item', $progress_task,
            'field_progress_status')
          ) {
            $stop = 'Stop';
            if (in_array($progress_status[0]['value'], array(
              DITOOLSI_TRAINING_PROGRESS_STATUS_NEW,
              DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST,
              DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW,
            ))) {
              $task_progress = node_load($progress_task->field_progress_task_progress[ LANGUAGE_NONE ][0]['target_id']);
              $task1         = node_load($task_progress->field_progress_task[ LANGUAGE_NONE ][0]['target_id']);
              $task_number   = 0;
              foreach ($lesson_item->tasks as $task) {
                if ($task->nid == $task1->nid) {
                  $valid_task_number = TRUE;
                  break;
                }
                $task_number++;
              }
              break;
            }
          }
        }

        if (empty($valid_task_number)) {
          $task_number = -1;
        }

        if (isset($progress->field_progress_status[ LANGUAGE_NONE ][0]['value'])) {
          $status = $progress->field_progress_status[ LANGUAGE_NONE ][0]['value'];
        }
        else {
          $status = -1;
        }

        if ($status == DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW) {
          drupal_set_message('Вы можете отправить урок на проверку');
          $has_actual_lesson = TRUE;
        }
        elseif ($status == DITOOLSI_TRAINING_PROGRESS_STATUS_NEW && $pupil->started) {
          drupal_set_message(t('You accessible task #@number, review the material and go through the homework. !link',
            array(
              '@number' => $task_number < 0 ? 1 : $task_number + 1,
              '!link'   => '<br>' . l(t('Go to a task'),
                  'course/' . $course->nid . '/' . $lesson->nid, array(
                    'fragment' => 'to-task-' . ($task_number < 0 ? 0 : $task_number),
                  )),
            )));
          user_cookie_save(array('next_lesson' => $lesson->nid));
          $has_actual_lesson = TRUE;
        }
        elseif ($status == DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST) {
          drupal_set_message(t('Teacher asks to complete your homework. !link', array(
            '!link' => '<br>' . l(t('Go to a task'), 'course/' . $course->nid . '/' . $lesson->nid,
                array(
                  'fragment' => 'to-task-' . $task_number,
                )),
          )));
          user_cookie_save(array('next_lesson' => $lesson->nid));
          $has_actual_lesson = TRUE;
        }
        break;
      }
      elseif ($pupil->started) {
        drupal_set_message(t('You accessible task #@number, review the material and go through the homework. !link',
          array(
            '@number' => 1,
            '!link'   => '<br>' . l(t('Go to a task'),
                'course/' . $course->nid . '/' . $lesson_item->nid, array(
                  'fragment' => 'to-task-0',
                )),
          )));
        user_cookie_save(array('next_lesson' => $lesson_item->nid));
        $has_actual_lesson = TRUE;
      }
      break;
    }
  }

  if (!$has_actual_lesson) {
    $query = db_select('ditoolsi_training_pupils_list', 'pl')
      ->fields('pl', array('current_lesson', 'course_id', 'user_id'))
      ->fields('lc', array('entity_id'))
      ->fields('ps', array('field_progress_status_value'))
      ->condition('pl.user_id', $user->uid)
      ->condition('pl.course_id', $course->nid, '<>');

    $or = db_or();
    $or->condition('ps.field_progress_status_value', array(
      DITOOLSI_TRAINING_PROGRESS_STATUS_NEW,
      DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST,
      DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW,
    ), 'IN')->isNull('ps.field_progress_status_value');

    $query->condition($or);
    $query->join('node', 'n', 'n.nid = pl.course_id');
    $query->join('field_data_field_lesson_course', 'lc',
      'lc.field_lesson_course_target_id = n.nid');
    $query->join('field_data_field_lesson_number', 'ln',
      'ln.entity_id = lc.entity_id AND ln.field_lesson_number_value = pl.current_lesson');
    $query->leftJoin('field_data_field_progress_lesson_id', 'pli',
      'pli.field_progress_lesson_id_target_id = lc.entity_id');
    $query->leftJoin('field_data_field_progress_status', 'ps', 'ps.entity_id = pli.entity_id');

    $object = $query->orderBy('ln.field_lesson_number_value', 'desc')
      ->distinct()
      ->execute()
      ->fetchObject();

    if ($object) {
      if (isset($status) && $status == DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW) {
        drupal_set_message(t('Your assignment for this course now checked. You can !link in other course',
          array(
            '!link' => l(t('begin the lessons'),
              'course/' . $object->course_id . '/' . $object->entity_id),
          )));
        unset($in_review);
      }
      elseif ($pupil) {
        drupal_set_message('Нет доступных к выполнению заданий на этом курсе. Вы можете <a href="' . url('course/' . $object->course_id . '/' . $object->entity_id) . '">приступить к выполнению уроков</a> на другом курсе');
        unset($in_review);
      }
    }
  }

  if (isset($status) && isset($in_review) && $status == DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW) {
    drupal_set_message(t('Your task now at checkout. Wait for the verification'));
  }
  $stop = 'Stop';

  return theme('ditoolsi_training_lesson_page', array(
    'node'           => $lesson,
    'wrapper'        => $wrapper,
    'course'         => $course,
    'account'        => $user,
    'lessons'        => $lessons,
    'current_lesson' => $current_lesson,
  ));
}

function ditoolsi_training_lesson_history_page($course, $lesson, $account) {

  drupal_add_css(drupal_get_path('theme', 'bootstrap_subtheme') . '/css/lesson.css');
  drupal_add_css(drupal_get_path('theme',
      'bootstrap_subtheme') . '/css/jquery/jquery.sliderTabs.min.css');
  drupal_add_js(drupal_get_path('theme', 'bootstrap_subtheme') . '/js/jquery/jquery.sliderTabs.js');
  drupal_add_js(drupal_get_path('module', 'ditoolsi_training') . '/js/lesson.js');

  $wrapper = entity_metadata_wrapper('node', $lesson);

  module_load_include('inc', 'ditoolsi_training', 'includes/form');

  $current_lesson = ditoolsi_training_get_current_lesson($course, $account);
  $lessons        = ditoolsi_training_get_lessons($course, $current_lesson, $account);
  $stop           = 'Stop';

  return theme('ditoolsi_training_lesson_page', array(
    'node'           => $lesson,
    'wrapper'        => $wrapper,
    'course'         => $course,
    'account'        => $account,
    'lessons'        => $lessons,
    'history'        => TRUE,
    'current_lesson' => $current_lesson,
  ));
}

function ditoolsi_training_requests_review_page($course) {
  global $user;

  $account = ditoolsi_profile_account($user);
  $nids    = array_keys($course->lessons);

  $query = db_select('node', 'n');
  $query->join('field_data_field_progress_lesson_id', 'lesson_id', 'lesson_id.entity_id = n.nid');
  $query->join('field_data_field_lesson_course', 'course',
    'course.entity_id = lesson_id.field_progress_lesson_id_target_id');
  $query->join('node', 'node_course', 'node_course.nid = course.field_lesson_course_target_id');
  $query->join('field_data_field_progress_status', 'progress_status',
    'progress_status.entity_id = n.nid');
  $query->condition('progress_status.field_progress_status_value',
    DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW)
    ->condition('node_course.nid', $course->nid)
    ->fields('n', array('nid'));

  $nids     = $query->execute()->fetchCol();
  $requests = node_load_multiple($nids);

  return theme('ditoolsi_training_requests_review_page', array(
    'node'     => $course,
    'wrapper'  => entity_metadata_wrapper('node', $course),
    'requests' => $requests,
  ));
}

function ditoolsi_training_check_lesson_page($course, $lesson, $progress) {

  drupal_add_css(drupal_get_path('theme', 'bootstrap_subtheme') . '/css/lesson.css');
  drupal_add_css(drupal_get_path('theme',
      'bootstrap_subtheme') . '/css/jquery/jquery.sliderTabs.min.css');
  drupal_add_js(drupal_get_path('theme', 'bootstrap_subtheme') . '/js/jquery/jquery.sliderTabs.js');
  drupal_add_js(drupal_get_path('module', 'ditoolsi_training') . '/js/lesson.js');

  module_load_include('inc', 'ditoolsi_training', 'includes/form');

  return theme('ditoolsi_training_review_page', array(
    'node'     => $lesson,
    'wrapper'  => entity_metadata_wrapper('node', $lesson),
    'course'   => $course,
    'progress' => $progress,
  ));
}

function ditoolsi_training_autotest_start_page($task) {
  global $user;

  $t_wrapper = entity_metadata_wrapper('node', $task);
  $lesson    = $t_wrapper->field_task_lesson->value();
  $course    = node_load($lesson->field_lesson_course[ LANGUAGE_NONE ][0]['target_id']);

  if (!ditoolsi_training_access('view course', $course, $lesson)) {
    return MENU_ACCESS_DENIED;
  }

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'auto_test_progress')
    ->propertyCondition('uid', $user->uid)
    ->fieldCondition('field_progress_task', 'target_id', $task->nid);

  $result = $query->execute();

  if (isset($result['node'])) {
    $nid           = key($result['node']);
    $progress_task = node_load($nid);
    $pt_wrapper    = entity_metadata_wrapper('node', $progress_task);
  }
  else {
    $progress_task                                = entity_create('node', array(
      'type'  => 'auto_test_progress',
      'uid'   => $user->uid,
      'title' => 'Progress of test [nid:' . $task->nid . '] for lesson [nid:' . $lesson->nid . '] for user [uid:' . $user->uid . ']',
    ));
    $progress_task->field_progress_task           = array(
      LANGUAGE_NONE => array(
        0 => array(
          'target_id' => $task->nid,
        ),
      ),
    );
    $progress_task->field_progress_start_autotest = array(
      LANGUAGE_NONE => array(
        0 => array(
          'value' => REQUEST_TIME,
        ),
      ),
    );
    $stop                                         = 'Stop';
  }

  $progress_task->field_progress_start_autotest = array(
    LANGUAGE_NONE => array(
      0 => array(
        'value' => REQUEST_TIME,
      ),
    ),
  );

  $pt_wrapper = entity_metadata_wrapper('node', $progress_task);
  $pt_wrapper->save();

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'lesson_progress')
    ->propertyCondition('uid', $user->uid)
    ->fieldCondition('field_progress_lesson_id', 'target_id', $lesson->nid);

  $result = $query->execute();

  if (isset($result['node'])) {
    $nid        = key($result['node']);
    $progress   = node_load($nid);
    $pl_wrapper = entity_metadata_wrapper('node', $progress);
  }
  else {
    $progress                           = entity_create('node', array(
      'type'  => 'lesson_progress',
      'uid'   => $user->uid,
      'title' => 'Progress of lesson [nid:' . $lesson->nid . '] for user [uid:' . $user->uid . ']',
    ));
    $progress->field_progress_lesson_id = array(
      LANGUAGE_NONE => array(
        0 => array(
          'target_id' => $lesson->nid,
        ),
      ),
    );
    $progress->field_progress_status    = array(
      LANGUAGE_NONE => array(
        0 => array(
          'value' => DITOOLSI_TRAINING_PROGRESS_STATUS_NEW,
        ),
      ),
    );
    $pl_wrapper                         = entity_metadata_wrapper('node', $progress);
    $pl_wrapper->save();
  }

  $items  = $pl_wrapper->field_progress_tasks->value();
  $exists = FALSE;

  foreach ($items as $item) {
    $fc_wrapper = entity_metadata_wrapper('field_collection_item', $item);
    if ($fc_wrapper->field_progress_task_progress->raw() == $pt_wrapper->getIdentifier()) {
      $exists = TRUE;
      $fc_wrapper->field_progress_status->set(DITOOLSI_TRAINING_PROGRESS_STATUS_NEW);
      $fc_wrapper->save();
      break;
    }
  }

  if (!$exists) {
    $fc_item = entity_create('field_collection_item', array(
      'field_name' => 'field_progress_tasks',
    ));
    $fc_item->setHostEntity('node', $progress);
    $fc_wrapper = entity_metadata_wrapper('field_collection_item', $fc_item);
    $fc_wrapper->field_progress_task_progress->set($pt_wrapper->getIdentifier());
    $fc_wrapper->field_progress_status->set(DITOOLSI_TRAINING_PROGRESS_STATUS_NEW);
    $fc_wrapper->save();
  }

  module_load_include('inc', 'ditoolsi_training', 'includes/form');

  $settings = array(
    'dt' => array(
      'startAutotest' => array(
        array(
          'taskId'      => $task->nid,
          'time'        => (int) $t_wrapper->field_autotest_time_completion->raw() * 60,
          'startedTime' => (int) $pt_wrapper->field_progress_start_autotest->raw(),
          'checkPath'   => url("task/{$task->nid}/check"),
        ),
      ),
    ),
  );

  $form       = drupal_get_form('ditoolsi_training_autotest_form', $lesson, $task);
  $commands   = array();
  $commands[] = ajax_command_replace('#autotest-' . $task->nid,
    '<div id="autotest-' . $task->nid . '">' . render($form) . '</div>');
  $commands[] = ajax_command_settings($settings, TRUE);
  $time       = REQUEST_TIME;
  ajax_deliver(array(
    '#type'     => 'ajax',
    '#commands' => $commands,
  ));
}

function ditoolsi_training_autotest_check_page($task) {
  global $user;

  $commands = array();

  $query = db_select('node', 'n');
  $query->join('field_data_field_progress_lesson_id', 'lesson_id', 'lesson_id.entity_id = n.nid');
  $query->join('field_data_field_progress_tasks', 'progress_tasks',
    'progress_tasks.entity_id = n.nid');
  $query->join('field_data_field_progress_task_progress', 'task_progress',
    'task_progress.entity_id = progress_tasks.field_progress_tasks_value');
  $query->join('node', 'node_progress_task',
    'node_progress_task.nid = task_progress.field_progress_task_progress_target_id');
  $query->join('field_data_field_progress_task', 'progress_task',
    'progress_task.entity_id = node_progress_task.nid');
  $query->join('node', 'node_task', 'node_task.nid = progress_task.field_progress_task_target_id');
  $query->join('field_data_field_autotest_time_completion', 'time_completion',
    'time_completion.entity_id = node_task.nid');
  $query->join('field_data_field_progress_start_autotest', 'start_autotest',
    'start_autotest.entity_id = node_progress_task.nid');

  $query->fields('node_progress_task', array('nid'))
    ->condition('n.type', 'lesson_progress')
    ->condition('node_progress_task.type', 'auto_test_progress')
    ->condition('node_task.type', 'lesson_auto_test')
    ->condition('node_task.nid', $task->nid)
    ->condition('n.uid', $user->uid)
    ->condition('node_progress_task.uid', $user->uid);

  $query->addField('time_completion', 'field_autotest_time_completion_value', 'time_completion');
  $query->addField('start_autotest', 'field_progress_start_autotest_value', 'start_autotest');

  $result          = $query->execute()->fetchObject();
  $time_completion = $result->time_completion * 60;

  if ((REQUEST_TIME - $time_completion) > $result->start_autotest) {
    $node                                                        = node_load($result->nid);
    $node->field_progress_blocked[ LANGUAGE_NONE ][0]['value']   = 1;
    $node->field_progress_time_lock[ LANGUAGE_NONE ][0]['value'] = REQUEST_TIME;
    node_save($node);

    $text       = ditoolsi_training_locked_autotest();
    $content    = '<div class="frozen-test">' . $text . '</div>';
    $commands[] = ajax_command_replace('#autotest-' . $task->nid,
      '<div id="autotest-' . $task->nid . '">' . $content . '</div>');
  }
  $stop = 'Stop';
  ajax_deliver(array(
    '#type'     => 'ajax',
    '#commands' => $commands,
  ));
}

function ditoolsi_training_to_review_page($course, $lesson) {
  global $user;

  $commands = array();

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'lesson_progress')
    ->propertyCondition('uid', $user->uid)
    ->fieldCondition('field_progress_lesson_id', 'target_id', $lesson->nid);

  $result = $query->execute();

  if (isset($result['node'])) {
    $nid          = key($result['node']);
    $node         = node_load($nid);
    $name         = '<b>' . ditoolsi_profile_name($user) . '</b>';
    $lesson_title = l($lesson->title, "course/{$course->nid}/{$lesson->nid}");
    $course_title = l($course->title, "course/{$course->nid}");
    $query        = db_select('ditoolsi_training_curators_list', 'tcl');
    $query->condition('tcl.course_id', $course->nid);
    $query->fields('tcl', array('course_id', 'user_id'));
    $result = $query->execute()->fetchAll();
    /*Check course option autocheck*/
    $final_lesson     = $lesson->field_lesson_final['und'][0]['value'];
    $lessons_count    = count($course->lessons);
    $lesson_number    = $lesson->field_lesson_number['und'][0]['value'];
    $lesson_autocheck = $lesson->field_lesson_check_type['und'][0]['value'];


    if (($lesson_autocheck == '1') && ($lesson_number != $lessons_count)) {
      // if (($lesson_autocheck == '1') && ($final_lesson == '0') && ($lesson_number != $lessons_count)) {

      $autocomments = $lesson->field_autocomment['und'][0]['value'];
      $autocomments = preg_split('/$\R?^/m', $autocomments);

      $lesson_low_score            = $lesson->field_low_score['und'][0]['value'];
      $lesson_high_score           = $lesson->field_high_score['und'][0]['value'];
      $bonus_points                = rand($lesson_low_score, $lesson_high_score);
      $progess_message             = $autocomments[ array_rand($autocomments) ];
      $progess_message             = str_replace(array(
        "\n",
        "\r\n",
        "\r",
      ), '', $progess_message);
      $stop                        = 'Stop';
      $node->field_progress_status = array(
        LANGUAGE_NONE => array(
          0 => array(
            'value' => DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED,
          ),
        ),
      );
      drupal_set_message(t('Урок был проверен учителем.'));
      user_cookie_save(array('new_lesson_available' => 'set'));

      ///print $_COOKIE['Drupal_visitor_key'];
      node_save($node);

      $text = $name . ' прислал отчет о выполненных заданиях урока ' . $lesson_title . ' с автопроверкой' . $course_title;

      foreach ($result as $key => $value) {
        $curator = user_load($value->user_id);
        ditoolsi_add_notification($curator, $text);
        $subject_curator = "{$user->mail} выслал задания на проверку";
        $message_curator = 'Вам отправлено задание на проверку ' . $lesson_title . '  от ученика ' . $user->mail . ' из курса ' . $course_title;
        drupal_mail('system', 'mail', $curator->mail, language_default(), array(
          'context' => array(
            'subject' => $subject_curator,
            'message' => $message_curator,
          ),
        ));
        $stop = 'Stop';
      }

      $teacher = ditoolsi_profile_account($course->uid);
      ditoolsi_add_notification($teacher, $text);

      _mgc_training_accept_lesson($node, $lesson, $bonus_points, $progess_message, $teacher);
      $text = 'Ваши задания урока ' . $lesson_title . ' курса ' . $course_title . ' отправлены на проверку куратору и будут скоро проверены.';
      ditoolsi_add_notification($user, $text);
      $stop = 'Stop';
      ctools_include('ajax');
      //$commands[] = ctools_ajax_command_reload();
      $commands[] = ctools_ajax_command_redirect('course/' . $course->nid);

    }
    else {
      $status = $node->field_progress_status[ LANGUAGE_NONE ][0]['value'];

      $node->field_progress_status = array(
        LANGUAGE_NONE => array(
          0 => array(
            'value' => DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW,
          ),
        ),
      );
      $p_wrapper                   = entity_metadata_wrapper('node', $node);

      foreach ($p_wrapper->field_progress_tasks->value() as $fc_item) {
        if ($fc_item->field_progress_status[ LANGUAGE_NONE ][0]['value'] != DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED) {
          $fc_item->field_progress_status[ LANGUAGE_NONE ][0]['value'] = DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW;
          $fc_item->save();
        }
      }

      $p_wrapper->field_progress_time_sent->set(REQUEST_TIME);

      node_save($node);

      ctools_include('ajax');
      $commands[] = ctools_ajax_command_reload();
      drupal_set_message(t('The lesson has been sent for review'));

      $name         = '<b>' . ditoolsi_profile_name($user) . '</b>';
      $lesson_title = l($lesson->title, "course/{$course->nid}/{$lesson->nid}/{$nid}/review");
      $course_title = l($course->title, "course/{$course->nid}");
      $query        = db_select('ditoolsi_training_curators_list', 'tcl');
      $query->condition('tcl.course_id', $course->nid);
      $query->fields('tcl', array('course_id', 'user_id'));
      $result = $query->execute()->fetchAll();

      if ($status == DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST) {
        $text = $name . ' Доработал задания урока ' . $lesson_title . ' курса ' . $course_title;
      }
      else {
        $text = $name . ' Прислал отчет о выполненных заданиях урока ' . $lesson_title . ' курса ' . $course_title;
      }

      foreach ($result as $key => $value) {
        $curator = user_load($value->user_id);
        ditoolsi_add_notification($curator, $text);
      }

      $teacher = ditoolsi_profile_account($course->uid);
      ditoolsi_add_notification($teacher, $text);

      if ($course->uid == DITOOLSI_FEEDBACK_UID) {
        $text = 'Ваши задания урока ' . $lesson_title . ' курса ' . $course_title . ' отправлены на проверку куратору и будут проверены в течении 3 дней.';
      }
      else {
        $text = 'Ваши задания урока ' . $lesson_title . ' курса ' . $course_title . ' отправлены на проверку куратору и будут проверены в течении 1 дня.';
      }

      ditoolsi_add_notification($user, $text);
    }
  }


  ajax_deliver(array(
    '#type'     => 'ajax',
    '#commands' => $commands,
  ));
}

function _mgc_training_accept_lesson(
  $progress,
  $lesson,
  $bonus_points,
  $progess_message,
  $teacher_account
) {
  global $user;

  $pupil_account                                                = $user;
  $progress->field_progress_status[ LANGUAGE_NONE ][0]['value'] = DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED;
  node_save($progress);
  $account = user_load($progress->uid);

  $flag = flag_get_user_flags('node', $lesson->nid, $progress->uid, $sid = 'student_next_lesson');

  if (!isset($flag['student_next_lesson'])) {
    $flag_finish = flag('flag', 'student_next_lesson', $lesson->nid, $progress, TRUE);
  }

  $course_id   = $lesson->field_lesson_course[ LANGUAGE_NONE ][0]['target_id'];
  $course      = node_load($lesson->field_lesson_course[ LANGUAGE_NONE ][0]['target_id']);
  $next_lesson = $lesson->field_lesson_number['und'][0]['value'];
  $stop        = 'Stop';

  db_update('ditoolsi_training_pupils_list')->fields(array(
    'current_lesson' => ($next_lesson + 1),
  ))->condition('user_id', $progress->uid)->condition('course_id', $course_id)->execute();

  $pl_wrapper = entity_metadata_wrapper('node', $progress);
  $pl_wrapper->field_lesson_progress_mark->set($bonus_points);
  $pl_wrapper->field_lesson_progress_message->set($progess_message);
  $pl_wrapper->save();

  if (module_exists('ditoolsi_bonuses')) {
    //Create 'Bonus points' node
    $node = entity_create('node', array(
      'type' => 'bonus_points',
      'uid'  => $user->uid,
    ));

    $title                             = t('Bonus points for completion lesson !lesson',
      array('!lesson' => $lesson->title,));
    $bp_wrapper                        = entity_metadata_wrapper('node', $node);
    $bp_wrapper->title                 = strip_tags($title);
    $bp_wrapper->field_charging_lesson = $lesson->nid;
    $bp_wrapper->field_charging_pupil  = $progress->uid;
    $bp_wrapper->field_charging_points = $bonus_points;
    $bp_wrapper->save();

    //Edit user's bonus account
    ditoolsi_bonuses_upd_course_bonus_balance($pupil_account, $course_id, $bonus_points);
  }

  $comment = strip_tags($progess_message);
  $comment = trim($comment);

  if ($comment) {
    _mgc_training_create_private_msg($teacher_account, $pupil_account, $progess_message);
  }
  /*Отправка писем*/
  mgc_send_mail_notification($bonus_points, $course, $account, $node, $comment);

  $access_token = isset($teacher_account->data['_vk_access_token']) ? $teacher_account->data['_vk_access_token'] : '';
  $stop         = 'Stop';
  if ($access_token && $comment && ($field_vk = field_get_items('user', $account,
      'field_vk_page'))
  ) {
    $parts = parse_url($field_vk[0]['value']);

    if ($bonus_points) {
      $comment .= isset($course->field_course_vk_notify_next_less['und'][0]['value']) ? $course->field_course_vk_notify_next_less['und'][0]['value'] : '';
      $comment .= '

---
Спасибо за выполненный урок, Ваше задание проверено и начислено ' . $bonus_points . ' бонусных баллов, можете приступать к следующему уроку на http://rewrite.up-lvl.ru/user

Первая Академия Фриланса
    ';
    }
    else {
      $comment .= isset($course->field_course_vk_notify_next_less['und'][0]['value']) ? $course->field_course_vk_notify_next_less['und'][0]['value'] : '';
      $comment .= '

---
Спасибо за выполненный урок, Ваше задание проверено, можете приступать к следующему уроку на http://rewrite.up-lvl.ru/user

Первая Академия Фриланса
    ';
    }

    if (isset($parts['path'])) {
      $parts['path'] = ltrim($parts['path'], '/');

      $comment = token_replace($comment, array(
        'node' => $course,
        'user' => $account,
      ));

      if (!empty($course->field_course_vk_notify_next_less[ LANGUAGE_NONE ][0]['value'])) {
        $comment = "Комментарий куратора: \n" . $comment;
        $comment = $course->field_course_vk_notify_next_less[ LANGUAGE_NONE ][0]['value'] . "\n\n" . $comment;
        $comment = format_string($comment, array(
          '@points'      => $bonus_points,
          '@lesson_link' => url("course/{$course->nid}/{$lesson->nid}", array('absolute' => TRUE)),
        ));
      }
      $stop = 'Stop';
      if ($access_token != NULL) {
        db_insert('ditoolsi_training_vk_messages_queue')->fields(array(
          'comment'      => $comment,
          'vk_id'        => $parts['path'],
          'access_token' => $access_token,
        ))->execute();
      }
    }
  }
  //notification rule
  $notification_options            = array();
  $course                          = node_load($course_id);
  $term                            = taxonomy_term_load(DT_TRAINING_TT_TRANSFER);
  $account                         = user_load($progress->uid);
  $notification_options['course']  = $course;
  $notification_options['account'] = $account;
  $notification_options['term']    = $term;

  rules_invoke_event('ditoolsi_training_pupils_notification', $notification_options);

  if ($bonus_points > 0) {
    ditoolsi_set_account_message($account,
      t('The teacher has put you an estimate made for @rate lesson', array(
        '@rate' => $bonus_points,
      )));
  }

  $query = db_select('node', 'n');
  $query->join('field_data_field_lesson_number', 'ln', 'ln.entity_id = n.nid');
  $query->join('field_data_field_lesson_course', 'lc', 'lc.entity_id = n.nid');
  $query->fields('n', array('nid'))
    ->condition('ln.field_lesson_number_value', ($next_lesson + 1))
    ->condition('lc.field_lesson_course_target_id', $course->nid)
    ->range(0, 1);

  $old_lesson_title = "<b>{$lesson->title}</b>";

  $nid    = $query->execute()->fetchField();
  $lesson = node_load($nid);
  $stop   = 'Stop';

  if ($lesson) {
    $name         = '<b>' . ditoolsi_profile_name($user) . '</b>';
    $lesson_title = l($lesson->title, "course/{$course->nid}/{$lesson->nid}");
    $course_title = l($course->title, "course/{$course->nid}");
    $rating_type  = field_get_items('node', $course, 'field_course_rating_type');

    if (isset($rating_type[0]['value']) && $rating_type[0]['value'] == DT_TRAINING_RATING_2) {
      $rate = $bonus_points ? 'зачет' : 'не зачет';
    }
    else {
      $rate = 'оценку ' . $bonus_points;
    }
    $stop         = 'Stop';
    $teacher_name = $teacher_account->field_last_name['und'][0]['value'] . ' ' . $teacher_account->field_first_name['und'][0]['value'];
    ditoolsi_add_notification($account,
      $teacher_name . ' поставил вам ' . $rate . ' по уроку ' . $old_lesson_title . ' и перевел вас на следующий урок ' . $lesson_title . ' курса ' . $course_title . '. Перейдите по ссылке с названием следующего урока, что бы посмотреть материалы.');
    // ditoolsi_add_notification($account, $name . ' перевел вас на следующий урок ' . $lesson_title . ' курса ' . $course_title);

  }
  user_cookie_save(array('new_lesson_available' => 'set'));

  // drupal_goto("course/{$course->nid}");
}

function _mgc_training_create_private_msg($usr_from, $usr_to, $message_text) {
  $usr_from = $usr_from->uid;
  $usr_to   = $usr_to->uid;

  $query = db_select('pm_index', 'i');
  $query->join('pm_message', 'm', 'i.mid = m.mid');
  $or = db_or();

  $and = db_and();
  $and->condition('i.recipient', $usr_from)->condition('m.author', $usr_to);
  $or->condition($and);

  $and = db_and();
  $and->condition('i.recipient', $usr_to)->condition('m.author', $usr_from);
  $or->condition($and);


  $query->condition($or);
  $query->fields('i', array('thread_id'));
  $results = $query->execute()->fetchObject();

  if (!empty($results)) {
    $mid = db_insert('pm_message')->fields(array(
      'author'    => $usr_from,
      'subject'   => '',
      'body'      => $message_text,
      'format'    => 'filtered_html',
      'timestamp' => time(),
    ))->execute();

    $query = db_insert('pm_index')->fields(array(
      'mid',
      'thread_id',
      'recipient',
      'is_new',
      'deleted',
      'type',
    ));
    $query->values(array(
      'mid'       => $mid,
      'thread_id' => $results->thread_id,
      'recipient' => $usr_from,
      'is_new'    => 0,
      'deleted'   => 0,
      'type'      => 'user',
    ));
    $query->values(array(
      'mid'       => $mid,
      'thread_id' => $results->thread_id,
      'recipient' => $usr_to,
      'is_new'    => 1,
      'deleted'   => 0,
      'type'      => 'user',
    ));
    $query->execute();
  }
  else {
    $mid = db_insert('pm_message')->fields(array(
      'author'    => $usr_from,
      'subject'   => '',
      'body'      => $message_text,
      'format'    => 'filtered_html',
      'timestamp' => time(),
    ))->execute();

    $query = db_insert('pm_index')->fields(array(
      'mid',
      'thread_id',
      'recipient',
      'is_new',
      'deleted',
      'type',
    ));
    $query->values(array(
      'mid'       => $mid,
      'thread_id' => $mid,
      'recipient' => $usr_from,
      'is_new'    => 0,
      'deleted'   => 0,
      'type'      => 'user',
    ));
    $query->values(array(
      'mid'       => $mid,
      'thread_id' => $mid,
      'recipient' => $usr_to,
      'is_new'    => 1,
      'deleted'   => 0,
      'type'      => 'user',
    ));
    $query->execute();
  }
}

function ditoolsi_training_start_learn_page($course) {
  global $user;

  ctools_include('ajax');

  $commands = array();
  $type     = $course->field_course_type[ LANGUAGE_NONE ][0]['value'];

  switch ($type) {
    // case DITOOLSI_TRAINING_COURSE_TYPE_PROMO:
    //   $account = ditoolsi_profile_account($user);

    //   $points = array(
    //     'bs' => 0,
    //     'br' => 0,
    //     'bk' => 0,
    //   );

    //   if ($bs = field_get_items('user', $account, 'field_bs')) {
    //     $points['bs'] = $bs[0]['value'];
    //   }
    //   if ($br = field_get_items('user', $account, 'field_br')) {
    //     $points['br'] = $br[0]['value'];
    //   }
    //   if ($bk = field_get_items('user', $account, 'field_bk')) {
    //     $points['bk'] = $bk[0]['value'];
    //   }

    //   $c_wrapper = entity_metadata_wrapper('node', $course);
    //   $requirements = array(
    //     'bs' => $c_wrapper->field_course_min_bs->value(),
    //     'br' => $c_wrapper->field_course_min_br->value(),
    //     'bk' => $c_wrapper->field_course_min_bk->value(),
    //   );

    //   $invalid = FALSE;
    //   foreach ($requirements as $type => $requirement) {
    //     if ($points[ $type ] < $requirement) {
    //       $invalid = TRUE;
    //       drupal_set_message($requirement);
    //       drupal_set_message($points[ $type ]);
    //       drupal_set_message(t('Your account does not meet the requirements'), 'error');
    //       break;
    //     }
    //   }

    //   if (!$invalid) {
    //     db_merge('ditoolsi_training_pupils_list')
    //       ->fields(array(
    //         'course_id'      => $course->nid,
    //         'user_id'        => $user->uid,
    //         'current_lesson' => 1,
    //         'created'        => REQUEST_TIME,
    //       ))
    //       ->key(array(
    //         'course_id' => $course->nid,
    //         'user_id'   => $user->uid,
    //       ))
    //       ->execute();
    //     drupal_set_message(t('Now you can learn in this course'));
    //   }

    //   $commands[] = ctools_ajax_command_redirect("course/{$course->nid}");
    //   break;

    case DITOOLSI_TRAINING_COURSE_TYPE_PAID:
      $commands[] = ctools_ajax_command_redirect("course/{$course->nid}/buy");
      break;
  }

  // $status = db_merge('ditoolsi_training_pupils_list')
  //   ->fields(array(
  //     'course_id'      => $course->nid,
  //     'user_id'        => $user->uid,
  //     'current_lesson' => 1,
  //     'created'        => REQUEST_TIME,
  //   ))
  //   ->key(array(
  //     'course_id' => $course->nid,
  //     'user_id'   => $user->uid,
  //   ))
  //   ->execute();

  // ctools_include('ajax');
  // $commands[] = ctools_ajax_command_redirect("course/{$course->nid}");

  // if ($status) {
  //   drupal_set_message(t('Now you can learn in this course'));
  // }

  ajax_deliver(array(
    '#type'     => 'ajax',
    '#commands' => $commands,
  ));
}

function ditoolsi_training_pupil_switch_lesson_content() {
  if (!empty($_POST)) {
    db_update('ditoolsi_training_pupils_list')
      ->fields(array('current_lesson' => $_POST['lesson_switch']))
      ->condition('course_id', $_POST['cID'])
      ->condition('user_id', $_POST['uID'])
      ->execute();

    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'node')
      ->entityCondition('bundle', 'lesson')
      ->fieldCondition('field_lesson_course', 'target_id', $_POST['cID'])
      ->fieldCondition('field_lesson_number', 'value', $_POST['lesson_switch']);

    $result = $query->execute();

    if (isset($result['node'])) {
      $lid    = key($result['node']);
      $lesson = node_load($lid);
      dt_training_update_start_time_progress($lesson, $_POST['uID']);
    }

    if (defined('DT_TRAINING_PROMO_COURSE_' . $_POST['cID'])) {
      $query = db_select('node', 'n');
      $query->join('field_data_field_progress_lesson_id', 'pli', 'pli.entity_id = n.nid');
      $query->join('field_data_field_lesson_course', 'lc',
        'lc.entity_id = pli.field_progress_lesson_id_target_id');
      $query->join('field_data_field_lesson_number', 'ln', 'ln.entity_id = lc.entity_id');
      $query->fields('n', array('nid'))
        ->condition('lc.field_lesson_course_target_id', $_POST['cID'])
        ->condition('n.uid', $_POST['uID'])
        ->condition('ln.field_lesson_number_value', $_POST['lesson_switch'], '>=');

      $nids  = $query->execute()->fetchCol();
      $nodes = node_load_multiple($nids);

      foreach ($nodes as $node) {
        foreach (field_get_items('node', $node, 'field_progress_tasks') as $value) {
          $fc_item = field_collection_item_load($value['value']);

          if (isset($fc_item->field_progress_task_progress[ LANGUAGE_NONE ][0]['target_id'])) {
            $nid = $fc_item->field_progress_task_progress[ LANGUAGE_NONE ][0]['target_id'];
            node_delete($nid);
          }
        }

        node_delete($node->nid);
      }
    }

    $response = array('status' => t('The pupil successfully transferred to a new lesson'));
    drupal_json_output($response);
    drupal_exit();
  }
  else {
    return MENU_NOT_FOUND;
  }
}

function ditoolsi_training_libraries_page() {
  global $user;

  $nodes = array();
  $items = array();
  $query = ditoolsi_training_get_libraries_query();

  if ($query) {
    $result = $query->execute()->fetchAllKeyed(0, 1);
    $nids   = array_keys($result);

    if (count($nids) == 1 && isset($user->roles[ DITOOLSI_PUPIL_RID ])) {
      drupal_goto('library/' . reset($nids));
    }

    $nodes = node_load_multiple($nids);
  }

  foreach ($nodes as $node) {
    $item = new stdClass();

    $item->wrapper = entity_metadata_wrapper('node', $node);
    $item->node    = $node;
    $item->picture = $item->wrapper->field_library_picture->value();
    $item->title   = $node->title;
    $item->cost    = $item->wrapper->field_library_cost->value();
    $item->courses = isset($result[ $node->nid ]) ? $result[ $node->nid ] : '';
    $items[]       = $item;
  }

  $tooltip     = '';
  $teacher     = ditoolsi_training_get_teacher();
  $description = isset($teacher->data['_description_libraries']) ? $teacher->data['_description_libraries'] : '';
  if ($description) {
    $description = check_plain($description);
  }
  elseif (user_access('ditoolsi edit description')) {
    $description = '<a href="#" id="edit-description">' . t('Edit description') . '</a>';
  }

  if (user_access('ditoolsi edit description')) {
    $tooltip = ' data-toggle="tooltip" data-placement="top" title="' . t('Click for edit') . '" data-access="1"';
  }

  $description = '<div class="description"' . $tooltip . '>' . $description . '</div>';
  $page        = "<div class='description-container'>$description</div>";

  if (ditoolsi_training_access('training toggle libraries')) {
    if (!empty($teacher->data['_disable_libraries'])) {
      $title = 'Enable libraries';
    }
    else {
      $title = 'Disable libraries';
    }

    $page .= '<div><br>' . l(t($title), 'toggle-libraries') . '</div>';
  }

  $page .= theme('ditoolsi_training_libraries_list', array(
    'items' => $items,
  ));

  drupal_add_js(array('descriptionType' => 'libraries'), 'setting');
  libraries_load('alertify.js');
  drupal_add_css(drupal_get_path('theme', 'bootstrap_subtheme') . '/css/libraries-list.css');

  return $page;
}

function dt_training_toggle_libraries_page() {
  global $user;

  $account                             = ditoolsi_profile_account($user);
  $account->data['_disable_libraries'] = empty($account->data['_disable_libraries']);
  user_save($account);
  drupal_goto('libraries');
}

function ditoolsi_training_library_page($library, $page = TRUE) {
  global $user;

  drupal_add_css(drupal_get_path('theme', 'bootstrap_subtheme') . '/css/libraries.css');
  drupal_add_js(drupal_get_path('module', 'ditoolsi_training') . '/js/library.js');

  $wrapper  = entity_metadata_wrapper('node', $library);
  $body     = $wrapper->body->value();
  $sections = ditoolsi_training_get_section($library->nid);
  if (isset($user->roles[ DITOOLSI_TEACHER_RID ]) || isset($user->roles[ DITOOLSI_ADMIN_RID ])) {
    $query = db_select('node', 'library');
    $query->leftJoin('field_data_field_library_courses', 'field_library_courses',
      'field_library_courses.entity_id = library.nid');
    $query->leftJoin('node', 'courses',
      'courses.nid = field_library_courses.field_library_courses_target_id');
    $query->addExpression('library.nid');
    $query->addExpression('GROUP_CONCAT(courses.title SEPARATOR \', \')');

    $query->groupBy('library.nid');
    $query->condition('library.nid', $library->nid)
      ->condition('library.type', 'library')
      ->condition('library.uid', $user->uid);

    $courses = $query->execute()->fetchAllKeyed(0, 1);
  }
  else {
    $query = db_select('node', 'library');
    $query->join('field_data_field_library_courses', 'courses', 'courses.entity_id = library.nid');
    $query->join('node', 'course', 'courses.field_library_courses_target_id = course.nid');

    $query->addExpression('library.nid');
    $query->addExpression('GROUP_CONCAT(course.title SEPARATOR \', \')');

    $query->groupBy('library.nid');
    $query->condition('library.nid', $library->nid);
    $courses = $query->execute()->fetchAllKeyed(0, 1);
  }

  $html = theme('ditoolsi_training_library_page', array(
    'node'        => $library,
    'title'       => $library->title,
    'description' => isset($body['value']) ? check_markup($body['value'], $body['format']) : '',
    'sections'    => $sections,
    'wrapper'     => $wrapper,
    'courses'     => $courses,
    'page'        => $page,
  ));


  return $html;
}

function ditoolsi_training_section_page($library, $section) {
  global $user;

  $commands = array();

  drupal_add_css(drupal_get_path('theme', 'bootstrap_subtheme') . '/css/section.css');
  drupal_add_css(drupal_get_path('theme',
      'bootstrap_subtheme') . '/css/jquery/jquery.sliderTabs.min.css');

  $wrapper = entity_metadata_wrapper('node', $section);

  $html = theme('ditoolsi_training_section_page', array(
    'node'    => $section,
    'wrapper' => $wrapper,
    'library' => $library,
  ));
  ctools_include('ajax');
  $commands[] = ajax_command_replace('#section', '<div id="section">' . $html . '</div>');

  // ajax_deliver(array('#type' => 'ajax', '#commands' => $commands));
  return [
    '#type'     => 'ajax',
    '#commands' => $commands,
  ];
}

/**
 * Function callback to page project team().
 */
function ditoolsi_training_project_team($course) {
  global $user;

  drupal_add_css(drupal_get_path('theme', 'bootstrap_subtheme') . '/css/course.css');
  drupal_add_js(drupal_get_path('theme',
      'bootstrap_subtheme') . '/js/jquery/jquery.readmore.min.js');
  drupal_add_js(drupal_get_path('module', 'ditoolsi_training') . '/js/course.js');

  $wrapper = entity_metadata_wrapper('node', $course);
  $body    = $wrapper->body->value();
  $autor   = user_load($course->uid);

  $query = db_select('ditoolsi_training_curators_list', 'tcl');
  $query->condition('tcl.course_id', $course->nid);
  $query->fields('tcl', array('course_id', 'user_id'));
  $result  = $query->execute()->fetchAll();
  $curator = array();

  foreach ($result as $key => $value) {
    $curator[] = user_load($value->user_id);
  }

  return theme('ditoolsi_training_project_team_page', array(
    'node'        => $course,
    'title'       => $course->title,
    'description' => isset($body['value']) ? check_markup($body['value'], $body['format']) : '',
    'lessons'     => NULL,
    'wrapper'     => $wrapper,
    'teacher'     => $autor,
    'curators'    => $curator,
  ));
}


/**
 * Function callback to page pupil statistic().
 */
function ditoolsi_training_pupil_achievements($course) {
  global $user;

  drupal_add_css(drupal_get_path('theme', 'bootstrap_subtheme') . '/css/course.css');
  drupal_add_css(drupal_get_path('theme', 'bootstrap_subtheme') . '/css/statistic.css');
  drupal_add_js(drupal_get_path('theme',
      'bootstrap_subtheme') . '/js/jquery/jquery.readmore.min.js');
  drupal_add_js(drupal_get_path('module', 'ditoolsi_training') . '/js/course.js');

  $wrapper = entity_metadata_wrapper('node', $course);
  $body    = $wrapper->body->value();
  $time    = array();
  $a       = 0;

  foreach ($course->lessons as $key => $value) {
    $flaggest = flag_get_user_flags('node', $key, $user->uid, $sid = NULL);
    if (!empty($flaggest['mark_first_time_student']) && !empty($flaggest['student_next_lesson'])) {
      $time[ $a ] = $flaggest['student_next_lesson']->timestamp - $flaggest['mark_first_time_student']->timestamp;
    }
    $a++;
  }

  $query = db_select('field_data_field_optimal_rate_learning', 'orl');
  $query->fields('orl', array('field_optimal_rate_learning_value'));
  $query->condition('orl.entity_id', $course->nid);
  $results = $query->execute()->fetchAll();
  foreach ($results as $o_key => $optimal_temp) {
  }

  if (isset($time)) {
    $count_time = count($time);
    $summ_time  = array_sum($time);
    if (!empty($count_time) && !empty($summ_time)) {
      $middle_time = $summ_time / $count_time;
      $middle_time = round($middle_time);
    }
    else {
      $middle_time = NULL;
    }
  }
  else {
    $count_time = NULL;
    $summ_time  = NULL;
  }

  $pupil = db_select('ditoolsi_training_pupils_list', 'pl')
    ->fields('pl', array('current_lesson', 'status'))
    ->condition('pl.user_id', $user->uid)
    ->condition('pl.course_id', $course->nid)
    ->execute()
    ->fetchObject();

  $tooltip     = '';
  $teacher     = ditoolsi_training_get_teacher();
  $description = isset($teacher->data['_description_statistic']) ? $teacher->data['_description_statistic'] : '';

  if ($description) {
    $description = check_plain($description);
  }
  elseif (user_access('ditoolsi edit description')) {
    $description = '<a href="#" id="edit-description">' . t('Edit description') . '</a>';
  }

  if (user_access('ditoolsi edit description')) {
    $tooltip = ' data-toggle="tooltip" data-placement="top" title="' . t('Click for edit') . '" data-access="1"';
  }

  $description = '<div class="description"' . $tooltip . '>' . $description . '</div>';
  $html        = "<div class='description-container'>$description</div>";

  drupal_add_js(array('descriptionType' => 'statistic'), 'setting');
  libraries_load('alertify.js');

  $html .= theme('ditoolsi_training_pupil_achievements', array(
    'course'       => $course,
    'wrapper'      => $wrapper,
    'description'  => isset($body['value']) ? check_markup($body['value'], $body['format']) : '',
    'pupil'        => $pupil,
    'count_time'   => $count_time,
    'middle_time'  => isset($middle_time) ? $middle_time : NULL,
    'optimal_temp' => isset($optimal_temp) ? $optimal_temp : NULL,
  ));

  return $html;
}

function ditoolsi_training_course_page_blocked($js = NULL) {

  module_load_include('inc', 'ditoolsi_training', 'includes/form');
  if (!$js) {
    return drupal_get_form('ditoolsi_training_course_blocked_form');
  }
  ctools_include('ajax');
  ctools_include('modal');
  $form_state = array(
    'ajax'       => TRUE,
    'title'      => t('Course blocked'),
    'build_info' => array(
      'args' => array(arg(2)),
    ),
  );
  $output     = ctools_modal_form_wrapper('ditoolsi_training_course_blocked_form', $form_state);
  print ajax_render($output);
  drupal_exit();
}

function ditoolsi_training_task_redaction_page($course, $lesson, $task) {
  global $user;

  module_load_include('inc', 'ditoolsi_training', 'includes/form');
  drupal_add_css(drupal_get_path('theme', 'bootstrap_subtheme') . '/css/lesson.css');
  drupal_add_js(drupal_get_path('module', 'ditoolsi_training') . '/js/task-redaction.js');
  drupal_add_js(array(
    'ditoolsi' => array(
      'pupil' => TRUE,
    ),
  ), 'setting');

  $history      = FALSE;
  $form         = array();
  $redactions   = array();
  $wrapper      = entity_metadata_wrapper('node', $lesson);
  $redactions[] = $task->nid . '/body';
  $body         = $task->body[ LANGUAGE_NONE ][0]['value'];
  $t_wrapper    = entity_metadata_wrapper('node', $task);
  $task_file    = $t_wrapper->field_text_task_file->value();

  if (!empty($task_file)) {
    $file_url = l($task_file['filename'], file_create_url($task_file['uri']), array(
      'attributes' => array(
        'target' => '_blank',
      ),
    ));
  }
  else {
    $file_url = t('None');
  }

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'text_task_progress')
    ->propertyCondition('uid', $user->uid)
    ->fieldCondition('field_progress_task', 'target_id', $task->nid);

  $result = $query->execute();

  if (isset($result['node'])) {
    $nid           = key($result['node']);
    $progress_task = node_load($nid);
    $lesson_status = ditoolsi_training_get_lesson_status($lesson, $user);
    if ($lesson_status == DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED || $lesson_status == DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW) {
      $history = TRUE;
    }
    $history    = ditoolsi_training_show_task_answer($task, $user);
    $pt_wrapper = entity_metadata_wrapper('node', $progress_task);
    foreach ($pt_wrapper->field_progress_comments as $key => $value) {
      $redactions[] = $nid . '/' . $key;
    }
  }

  if (!$history) {
    $form = drupal_get_form('ditoolsi_training_save_task_redaction_form', $lesson, $task);
  }
  else {
    $form = drupal_get_form('ditoolsi_training_task_redaction_form', $task, 'body');
  }

  drupal_add_library('system', 'drupal.ajax');

  return theme('ditoolsi_training_task_redaction_page', array(
    'node'       => $lesson,
    'course'     => $course,
    'form'       => $form,
    'wrapper'    => $wrapper,
    'history'    => $history,
    'account'    => $user,
    'redactions' => $redactions,
    'body'       => $body,
    'file_url'   => $file_url,
  ));
}

function ditoolsi_training_review_task_redaction_page(
  $course,
  $lesson,
  $progress,
  $task
) {
  module_load_include('inc', 'ditoolsi_training', 'includes/form');
  drupal_add_css(drupal_get_path('theme', 'bootstrap_subtheme') . '/css/lesson.css');
  drupal_add_js(drupal_get_path('module', 'ditoolsi_training') . '/js/task-redaction.js');
  drupal_add_js(array(
    'ditoolsi' => array(
      'pupil' => FALSE,
    ),
  ), 'setting');

  $account      = user_load($progress->uid);
  $history      = FALSE;
  $form         = array();
  $redactions   = array();
  $wrapper      = entity_metadata_wrapper('node', $lesson);
  $redactions[] = $task->nid . '/body';
  $pupil        = ditoolsi_profile_name($account);
  $body         = $task->body[ LANGUAGE_NONE ][0]['value'];

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'text_task_progress')
    ->propertyCondition('uid', $account->uid)
    ->fieldCondition('field_progress_task', 'target_id', $task->nid);

  $result = $query->execute();

  $nid             = key($result['node']);
  $progress_task   = node_load($nid);
  $progress_status = ditoolsi_training_get_task_progress($task, $account);
  $lesson_complete = ditoolsi_training_is_lesson_complete($lesson, $account);
  if ($progress_status == DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED && $lesson_complete) {
    $history = TRUE;
  }
  $pt_wrapper = entity_metadata_wrapper('node', $progress_task);
  foreach ($pt_wrapper->field_progress_comments as $key => $value) {
    $redactions[] = $nid . '/' . $key;
  }


  if (!$history) {
    $form = drupal_get_form('ditoolsi_training_save_task_redaction_form', $lesson, $task,
      $progress);
  }
  else {
    $form = drupal_get_form('ditoolsi_training_task_redaction_form', $task, 'body');
  }

  return theme('ditoolsi_training_task_redaction_page', array(
    'node'       => $lesson,
    'course'     => $course,
    'form'       => $form,
    'wrapper'    => $wrapper,
    'history'    => $history,
    'account'    => $account,
    'redactions' => $redactions,
    'pupil'      => $pupil,
    'body'       => $body,
    // 'file_url'       => $file_url,
  ));
}

function ditoolsi_training_task_redaction($task, $redaction) {
  drupal_add_library('system', 'drupal.ajax');
  module_load_include('inc', 'ditoolsi_training', 'includes/form');

  $form_redaction = drupal_get_form('ditoolsi_training_task_redaction_form', $task, $redaction);

  $commands   = array();
  $commands[] = ajax_command_replace('#task-redaction',
    '<div id="task-redaction">' . render($form_redaction) . '</div>');

  $page = array(
    '#type'     => 'ajax',
    '#commands' => $commands,
  );

  ajax_deliver($page);
}

function dt_training_promo_course_check_page() {
  drupal_add_http_header('Access-Control-Allow-Origin', '*');

  $response = array();
  $valid    = TRUE;

  extract($_POST);

  if (empty($name)) {
    $response['error']['.form-item-name'] = 'Поле обязательно для заполнения!';
    $valid                                = FALSE;
  }

  if (empty($mail) || !valid_email_address($mail)) {
    $response['error']['.form-item-mail'] = 'E-mail не валидный!';
    $valid                                = FALSE;
  }
  $stop = 'Stop';
  /*Убираем, так как не нужен больше телефон*/
  /* if (empty($phone)) {
     $response['error']['.form-item-phone'] = 'Поле обязательно для заполнения!';
     $valid                                 = FALSE;
   }*/

  if (empty($vk)) {
    $response['error']['.form-item-vk'] = 'Поле обязательно для заполнения!';
    $valid                              = FALSE;
  }

  if (empty($course_id)) {
    watchdog(__FUNCTION__, 'empty course_id');
    print 'empty course_id';

    return;
  }

  if (!node_load($course_id)) {
    watchdog(__FUNCTION__, 'course not exists');
    print 'course not exists';

    return;
  }

  if (!defined('DT_TRAINING_PROMO_COURSE_' . $course_id)) {
    watchdog(__FUNCTION__, 'invalid course_id');
    print 'invalid course_id';

    return;
  }

  $parts = parse_url($vk);

  if (!(isset($parts['host']) && ($parts['host'] == 'vk.com' || $parts['host'] == 'vkontakte.ru' || $parts['host'] == 'new.vk.com'))) {
    $response['error']['.form-item-vk'] = 'Введите правильную ссылку на страницу Вконтакте';
  }

  if ($valid) {
    if (user_load_by_mail($mail)) {
      $response['error']['.form-item-mail'] = t('User with this email already exists') . '<br>' . t('If this is your account, you can enter the site by clicking on the link: !link',
          array(
            '!link' => l(url('user/login', array(
              'absolute' => TRUE,
            )), 'user/login', array(
              'absolute'   => TRUE,
              'attributes' => array(
                'target' => '_blank',
              ),
            )),
          ));
    }
    else {
      if (isset($_GET['q'])) {
        unset($_GET['q']);
      }

      $response['redirect'] = url('promo-course/register', array(
        'absolute' => TRUE,
        'query'    => $_POST,
      ));
    }
  }

  drupal_json_output($response);
}

function dt_training_ege_course_check_page() {
  drupal_add_http_header('Access-Control-Allow-Origin', '*');

  $response = array();
  $valid    = TRUE;

  extract($_POST);

  if (empty($name)) {
    $response['error']['.form-item-name'] = 'Поле обязательно для заполнения!';
    $valid                                = FALSE;
  }

  if (empty($mail) || !valid_email_address($mail)) {
    $response['error']['.form-item-mail'] = 'E-mail не валидный!';
    $valid                                = FALSE;
  }

  if (empty($phone)) {
    $response['error']['.form-item-phone'] = 'Поле обязательно для заполнения!';
    $valid                                 = FALSE;
  }

  if (empty($vk)) {
    $response['error']['.form-item-vk'] = 'Поле обязательно для заполнения!';
    $valid                              = FALSE;
  }

  if (empty($course_id)) {
    watchdog(__FUNCTION__, 'empty course_id');
    print 'empty course_id';

    return;
  }

  if (!node_load($course_id)) {
    watchdog(__FUNCTION__, 'course not exists');
    print 'course not exists';

    return;
  }

  if (!defined('DT_TRAINING_EGE_COURSE_' . $course_id)) {
    watchdog(__FUNCTION__, 'invalid course_id');
    print 'invalid course_id';

    return;
  }

  $parts = parse_url($vk);

  if (!(isset($parts['host']) && ($parts['host'] == 'vk.com' || $parts['host'] == 'vkontakte.ru' || $parts['host'] == 'new.vk.com'))) {
    $response['error']['.form-item-vk'] = 'Введите правильную ссылку на страницу Вконтакте';
  }

  if ($valid) {
    if (user_load_by_mail($mail)) {
      $response['error']['.form-item-mail'] = t('User with this email already exists') . '<br>' . t('If this is your account, you can enter the site by clicking on the link: !link',
          array(
            '!link' => l(url('user/login', array(
              'absolute' => TRUE,
            )), 'user/login', array(
              'absolute'   => TRUE,
              'attributes' => array(
                'target' => '_blank',
              ),
            )),
          ));
    }
    else {
      if (isset($_GET['q'])) {
        unset($_GET['q']);
      }

      $response['redirect'] = url('ege-course/register', array(
        'absolute' => TRUE,
        'query'    => $_POST,
      ));
    }
  }

  drupal_json_output($response);
}

function dt_training_ege_course_register_page() {
  global $user;

  if (user_is_logged_in()) {
    drupal_goto('user');
  }

  extract($_GET);

  if (empty($name)) {
    watchdog(__FUNCTION__, 'empty name');
    print 'empty name';

    return;
  }
  elseif (empty($mail) || !valid_email_address($mail)) {
    watchdog(__FUNCTION__, 'empty mail');
    print 'empty mail';

    return;
  } /* elseif (empty($phone)) {
     watchdog(__FUNCTION__, 'empty phone');
     print 'empty phone';

     return;
   }*/
  elseif (empty($course_id)) {
    watchdog(__FUNCTION__, 'empty course_id');
    print 'empty course_id';

    return;
  }
  elseif (!node_load($course_id)) {
    watchdog(__FUNCTION__, 'course not exists');
    print 'course not exists';

    return;
  }
  elseif (!defined('DT_TRAINING_EGE_COURSE_' . $course_id)) {
    watchdog(__FUNCTION__, 'invalid course_id');
    print 'invalid course_id';

    return;
  }

  $roles  = user_roles();
  $fields = array(
    'name'             => $mail,
    'mail'             => $mail,
    'pass'             => DITOOLSI_DEFAULT_PASS,
    'status'           => 1,
    'init'             => $mail,
    'roles'            => array(DITOOLSI_PUPIL_RID => $roles[ DITOOLSI_PUPIL_RID ]),
    'access'           => REQUEST_TIME,
    'field_first_name' => array(
      LANGUAGE_NONE => array(
        0 => array(
          'value' => $name,
        ),
      ),
    ),
    'field_phone'      => array(
      LANGUAGE_NONE => array(
        0 => array(
          'value' => '',
        ),
      ),
    ),
  );

  if (!empty($vk)) {
    $parts = parse_url($vk);

    if (isset($parts['host']) && ($parts['host'] == 'vk.com' || $parts['host'] == 'vkontakte.ru' || $parts['host'] == 'new.vk.com')) {
      $fields['field_vk_page'] = array(
        LANGUAGE_NONE => array(
          0 => array(
            'value' => $vk,
          ),
        ),
      );

      $message = '
' . $name . ' от лица команды egena89 рад приветствовать Вас в команде :)

Я Ваш куратор, можете добавить меня в друзья, буду помогать Вам в обучении и отвечать на все вопросы!

Ваш логин: ' . $fields['mail'] . '
Ваш пароль: 1
Ссылка для входа: http://rewrite.up-lvl.ru/user

Вы уже можете приступать к обучению, первый урок доступен в Вашем личном кабинете.

Советую Вам поменять пароль при следующем посещении Профологии.

Так же, для успешного прохождения обучения посмотрите видео: https://www.youtube.com/watch?v=l7ENpHwMcZI&list=PLBeQPS5Ebv3HLci17XLUnj6Pcce5sP8ci

Желаем Вам успешного прохождения Курса! Ваша Академия.
';

      $course       = node_load($course_id);
      $teacher      = user_load($course->uid);
      $access_token = isset($teacher->data['_vk_access_token']) ? $teacher->data['_vk_access_token'] : '';

      db_insert('ditoolsi_training_vk_messages_queue')->fields(array(
        'comment'      => $message,
        'vk_id'        => $vk,
        'access_token' => $access_token,
      ))->execute();
    }
  }

  //require_once(DRUPAL_ROOT . '/' . drupal_get_path('module', 'ditoolsi_profile') . '/bitrix24Lead.class.php');

  //$bitrix24Lead = new bitrix24Lead(BITRIX24_LOGIN, BITRIX24_PASSWORD, BITRIX24_PREFIX);
  $node = node_load($course_id);
//  $bitrix24Lead
//      ->setTitle('Регистрация на промокурс - ' . $node->title)
//      ->setValue('NAME', $name)
//      ->setValue('PHONE_MOBILE', $phone)
//      ->setValue('EMAIL_HOME', $mail)
//      ->assign(12)
//      ->execute();

  user_save('', $fields);
  $lesson_id = reset($node->lessons)->nid;

  if ($uid = user_authenticate($mail, DITOOLSI_DEFAULT_PASS)) {
    db_insert('ditoolsi_training_pupils_list')->fields(array(
      'course_id'      => $course_id,
      'user_id'        => $uid,
      'current_lesson' => 1,
      'created'        => REQUEST_TIME,
      'status'         => DITOOLSI_TRAINING_PUPIL_ACTIVE,
    ))->execute();

    $user = user_load($uid);
    user_login_finalize();
    user_save($user);
    drupal_set_message(t('Congratulations!!! You have successfully signed up on the site. '),
      'status');
    $target = $find = '';

    module_load_include('php', 'ditoolsi', 'classes/GetResponseAPI.class');
    ditoolsi_profile_send_mail_teacher($name, $mail, $phone, $target, $find);
    // ditoolsi_training_send_mail_to_pupil($account, $node->nid);
    $api      = new GetResponse('d02fb8f492a44b6186984071ac219a8d');
    $campaign = $api->getCampaignByName('ege_lending');
    $api->addContact($campaign, $name, $mail, 'standard', 0, array(
      'custom_phone1' => $phone,
    ));
    drupal_goto('course/' . $course_id . '/' . $lesson_id);
  }
}

function dt_training_socfit_course_check_page() {
  drupal_add_http_header('Access-Control-Allow-Origin', '*');

  $response = array();
  $valid    = TRUE;

  extract($_POST);

  if (empty($name)) {
    $response['error']['.form-item-name'] = 'Поле обязательно для заполнения!';
    $valid                                = FALSE;
  }

  if (empty($mail) || !valid_email_address($mail)) {
    $response['error']['.form-item-mail'] = 'E-mail не валидный!';
    $valid                                = FALSE;
  }

  if (empty($phone)) {
    $response['error']['.form-item-phone'] = 'Поле обязательно для заполнения!';
    $valid                                 = FALSE;
  }

  if (empty($vk)) {
    $response['error']['.form-item-vk'] = 'Поле обязательно для заполнения!';
    $valid                              = FALSE;
  }

  if (empty($course_id)) {
    watchdog(__FUNCTION__, 'empty course_id');
    print 'empty course_id';

    return;
  }

  if (!node_load($course_id)) {
    watchdog(__FUNCTION__, 'course not exists');
    print 'course not exists';

    return;
  }

  if (!defined('DT_TRAINING_SOCFIT_COURSE_' . $course_id)) {
    watchdog(__FUNCTION__, 'invalid course_id');
    print 'invalid course_id';

    return;
  }

  $parts = parse_url($vk);

  if (!(isset($parts['host']) && ($parts['host'] == 'vk.com' || $parts['host'] == 'vkontakte.ru' || $parts['host'] == 'new.vk.com'))) {
    $response['error']['.form-item-vk'] = 'Введите правильную ссылку на страницу Вконтакте';
  }

  if ($valid) {
    if (user_load_by_mail($mail)) {
      $response['error']['.form-item-mail'] = t('User with this email already exists') . '<br>' . t('If this is your account, you can enter the site by clicking on the link: !link',
          array(
            '!link' => l(url('user/login', array(
              'absolute' => TRUE,
            )), 'user/login', array(
              'absolute'   => TRUE,
              'attributes' => array(
                'target' => '_blank',
              ),
            )),
          ));
    }
    else {
      if (isset($_GET['q'])) {
        unset($_GET['q']);
      }

      $response['redirect'] = url('socfit-course/register', array(
        'absolute' => TRUE,
        'query'    => $_POST,
      ));
    }
  }

  drupal_json_output($response);
}

function dt_training_socfit_course_register_page() {
  global $user;

  if (user_is_logged_in()) {
    drupal_goto('user');
  }

  extract($_GET);

  if (empty($name)) {
    watchdog(__FUNCTION__, 'empty name');
    print 'empty name';

    return;
  }
  elseif (empty($mail) || !valid_email_address($mail)) {
    watchdog(__FUNCTION__, 'empty mail');
    print 'empty mail';

    return;
  } /*elseif (empty($phone)) {
    watchdog(__FUNCTION__, 'empty phone');
    print 'empty phone';

    return;
  }*/
  elseif (empty($course_id)) {
    watchdog(__FUNCTION__, 'empty course_id');
    print 'empty course_id';

    return;
  }
  elseif (!node_load($course_id)) {
    watchdog(__FUNCTION__, 'course not exists');
    print 'course not exists';

    return;
  }
  elseif (!defined('DT_TRAINING_SOCFIT_COURSE_' . $course_id)) {
    watchdog(__FUNCTION__, 'invalid course_id');
    print 'invalid course_id';

    return;
  }

  $roles  = user_roles();
  $fields = array(
    'name'             => $mail,
    'mail'             => $mail,
    'pass'             => DITOOLSI_DEFAULT_PASS,
    'status'           => 1,
    'init'             => $mail,
    'roles'            => array(DITOOLSI_PUPIL_RID => $roles[ DITOOLSI_PUPIL_RID ]),
    'access'           => REQUEST_TIME,
    'field_first_name' => array(
      LANGUAGE_NONE => array(
        0 => array(
          'value' => $name,
        ),
      ),
    ),
    'field_phone'      => array(
      LANGUAGE_NONE => array(
        0 => array(
          'value' => '',
        ),
      ),
    ),
  );

  if (!empty($vk)) {
    $parts = parse_url($vk);

    if (isset($parts['host']) && ($parts['host'] == 'vk.com' || $parts['host'] == 'vkontakte.ru' || $parts['host'] == 'new.vk.com')) {
      $fields['field_vk_page'] = array(
        LANGUAGE_NONE => array(
          0 => array(
            'value' => $vk,
          ),
        ),
      );

      $message = '
' . $name . ' от лица SOCFIT рад приветствовать Вас в команде :)

Я Ваш куратор, можете добавить меня в друзья, буду помогать Вам в обучении и отвечать на все вопросы!

Ваш логин: ' . $fields['mail'] . '
Ваш пароль: 1
Ссылка для входа: http://rewrite.up-lvl.ru/user

Вы уже можете приступать к обучению, первый урок доступен в Вашем личном кабинете.

Советую Вам поменять пароль при следующем посещении Профологии.

Так же, для успешного прохождения обучения посмотрите видео: https://www.youtube.com/watch?v=l7ENpHwMcZI&list=PLBeQPS5Ebv3HLci17XLUnj6Pcce5sP8ci

Желаем Вам успешного прохождения Курса! Ваша Академия.
';

      $course       = node_load($course_id);
      $teacher      = user_load($course->uid);
      $access_token = isset($teacher->data['_vk_access_token']) ? $teacher->data['_vk_access_token'] : '';

      db_insert('ditoolsi_training_vk_messages_queue')->fields(array(
        'comment'      => $message,
        'vk_id'        => $vk,
        'access_token' => $access_token,
      ))->execute();
    }
  }

  //require_once(DRUPAL_ROOT . '/' . drupal_get_path('module', 'ditoolsi_profile') . '/bitrix24Lead.class.php');

  //$bitrix24Lead = new bitrix24Lead(BITRIX24_LOGIN, BITRIX24_PASSWORD, BITRIX24_PREFIX);
  $node = node_load($course_id);
//  $bitrix24Lead
//      ->setTitle('Регистрация на промокурс - ' . $node->title)
//      ->setValue('NAME', $name)
//      ->setValue('PHONE_MOBILE', $phone)
//      ->setValue('EMAIL_HOME', $mail)
//      ->assign(12)
//      ->execute();

  user_save('', $fields);
  $lesson_id = reset($node->lessons)->nid;

  if ($uid = user_authenticate($mail, DITOOLSI_DEFAULT_PASS)) {
    db_insert('ditoolsi_training_pupils_list')->fields(array(
      'course_id'      => $course_id,
      'user_id'        => $uid,
      'current_lesson' => 1,
      'created'        => REQUEST_TIME,
      'status'         => DITOOLSI_TRAINING_PUPIL_ACTIVE,
    ))->execute();

    $user = user_load($uid);
    user_login_finalize();
    user_save($user);
    drupal_set_message(t('Congratulations!!! You have successfully signed up on the site. '),
      'status');
    $target = $find = '';

    module_load_include('php', 'ditoolsi', 'classes/GetResponseAPI.class');
    ditoolsi_profile_send_mail_teacher($name, $mail, '', $target, $find);
    // ditoolsi_training_send_mail_to_pupil($account, $node->nid);
    $api      = new GetResponse('d02fb8f492a44b6186984071ac219a8d');
    $campaign = $api->getCampaignByName('fitnes_2');
    $response = $api->addContact($campaign, $name, $mail, 'standard', 0, array(
      'custom_phone' => '',
    ));
    drupal_goto('course/' . $course_id . '/' . $lesson_id);
  }
}

function dt_training_promo_course_register_page() {
  global $user;

  if (user_is_logged_in()) {
    drupal_goto('user');
  }

  extract($_GET);

  if (empty($name)) {
    watchdog(__FUNCTION__, 'empty name');
    print 'empty name';

    return;
  }
  elseif (empty($mail) || !valid_email_address($mail)) {
    watchdog(__FUNCTION__, 'empty mail');
    print 'empty mail';

    return;
  } /* elseif (empty($phone)) {
     watchdog(__FUNCTION__, 'empty phone');
     print 'empty phone';

     return;
   }*/
  elseif (empty($course_id)) {
    watchdog(__FUNCTION__, 'empty course_id');
    print 'empty course_id';

    return;
  }
  elseif (!node_load($course_id)) {
    watchdog(__FUNCTION__, 'course not exists');
    print 'course not exists';

    return;
  }
  elseif (!defined('DT_TRAINING_PROMO_COURSE_' . $course_id)) {
    watchdog(__FUNCTION__, 'invalid course_id');
    print 'invalid course_id';

    return;
  }

  $roles  = user_roles();
  $fields = array(
    'name'             => $mail,
    'mail'             => $mail,
    'pass'             => DITOOLSI_DEFAULT_PASS,
    'status'           => 1,
    'init'             => $mail,
    'roles'            => array(DITOOLSI_PUPIL_RID => $roles[ DITOOLSI_PUPIL_RID ]),
    'access'           => REQUEST_TIME,
    'field_first_name' => array(
      LANGUAGE_NONE => array(
        0 => array(
          'value' => $name,
        ),
      ),
    ),
    'field_phone'      => array(
      LANGUAGE_NONE => array(
        0 => array(
          'value' => '',
        ),
      ),
    ),
  );

  $node         = node_load($course_id);
  $teacher      = user_load($node->uid);
  $access_token = isset($teacher->data['_vk_access_token']) ? $teacher->data['_vk_access_token'] : '';

  if (!empty($vk)) {
    $parts = parse_url($vk);

    if (isset($parts['host']) && ($parts['host'] == 'vk.com' || $parts['host'] == 'vkontakte.ru' || $parts['host'] == 'new.vk.com')) {
      $fields['field_vk_page'] = array(
        LANGUAGE_NONE => array(
          0 => array(
            'value' => $vk,
          ),
        ),
      );

      $comment = '
От лица Первой Академии Фриланса рада приветствовать Вас в команде :) 

Я Ваш куратор, можете добавить меня в друзья, буду помогать Вам в обучении и отвечать на все вопросы! 

Ваш логин: ' . $fields['mail'] . '
Ваш пароль: 1 
Ссылка для входа: http://rewrite.up-lvl.ru/user 

Вы уже можете приступать к обучению, первый урок доступен в Вашем личном кабинете: 

"Основное отличие фрилансера от наемного работника, Плюсы и минусы фриланса!" 

Советую Вам поменять пароль при следующем посещении Профологии. 

Так же, для успешного прохождения обучения посмотрите видео: https://www.youtube.com/watch?v=l7ENpHwMcZI&list=PLBeQPS5Ebv3HLci17XLUnj6Pcce5sP8ci  

Желаем Вам успешного прохождения Курса! Ваша Академия.';

      if (isset($parts['path']) && $access_token && user_access('training notify vk', $teacher)) {
        $parts['path'] = ltrim($parts['path'], '/');
        if ($access_token != NULL) {
          db_insert('ditoolsi_training_vk_messages_queue')->fields(array(
            'comment'      => $comment,
            'vk_id'        => $parts['path'],
            'access_token' => $access_token,
          ))->execute();
        }
      }
    }
  }

  require_once(DRUPAL_ROOT . '/' . drupal_get_path('module',
      'ditoolsi_profile') . '/bitrix24Lead.class.php');

  $bitrix24Lead = new bitrix24Lead(BITRIX24_LOGIN, BITRIX24_PASSWORD, BITRIX24_PREFIX);
  $bitrix24Lead->setTitle('Регистрация на промокурс - ' . $node->title)
    ->setValue('NAME', $name)
    ->setValue('PHONE_MOBILE', '')
    ->setValue('EMAIL_HOME', $mail)
    ->assign(12)
    ->execute();

  user_save('', $fields);
  $lesson_id = reset($node->lessons)->nid;

  if ($uid = user_authenticate($mail, DITOOLSI_DEFAULT_PASS)) {
    db_insert('ditoolsi_training_pupils_list')->fields(array(
      'course_id'      => $course_id,
      'user_id'        => $uid,
      'current_lesson' => 1,
      'created'        => REQUEST_TIME,
      'status'         => DITOOLSI_TRAINING_PUPIL_ACTIVE,
    ))->execute();

    $user = user_load($uid);
    user_login_finalize();
    $account       = user_save($user);
    $first_task_id = $node->lessons[ $lesson_id ]->tasks;
    $first_task_id = reset($first_task_id);
    //$stop = 'Stop';
    drupal_set_message(t('Поздравляем! Вам уже доступен для изучения курс "@course_title". <br>
Сейчас Вам нужно посмотреть первый видеоурок и выполнить !task.<br>
Мы желаем Вам успехов в освоении курса! Пора сделать первый шаг))', array(
      '@course_title' => $node->title,
      '!link'         => l(t('первый видеоурок'), 'course/' . $node->nid . '/' . $lesson_id),
      '!task'         => l(t('первые задания к уроку'), '', array(
        'external' => TRUE,
        'fragment' => 'to-task-0',
      )),

    )));
    //  drupal_set_message(t('Congratulations!!! You have successfully signed up on the site. '), 'status');
    $target = $find = '';

    module_load_include('php', 'ditoolsi', 'classes/GetResponseAPI.class');
    ditoolsi_profile_send_mail_teacher($name, $mail, '', $target, $find);
    ditoolsi_training_send_mail_to_pupil($account, $node->nid);
    $api      = new GetResponse('d02fb8f492a44b6186984071ac219a8d');
    $campaign = $api->getCampaignByName('paf_promo_course');
    $api->addContact($campaign, $name, $mail, 'standard', 0, array(
      'telefon_text' => '',
    ));
    mgc_start_learn2($node);
    drupal_goto('course/' . $course_id . '/' . $lesson_id);
  }
}

function ditoolsi_training_library_buy_page($node) {
  global $user;

  $token = isset($_GET['token']) ? $_GET['token'] : FALSE;

  if (!drupal_valid_token($token, 'buy-library-' . $node->nid . '-' . session_id())) {
    return MENU_ACCESS_DENIED;
  }

  if (($cost = field_get_items('node', $node, 'field_library_cost')) && !empty($cost[0]['value'])) {
    module_load_include('inc', 'ditoolsi_training', 'includes/form');

    return drupal_get_form('ditoolsi_training_library_buy_form', $node);
  }

  db_insert('ditoolsi_training_library_pupils')->fields(array(
    'user_id'    => $user->uid,
    'library_id' => $node->nid,
    'created'    => REQUEST_TIME,
  ))->execute();

  drupal_set_message(t('You have become a party to the library'));
  drupal_goto('library/' . $node->nid);
}

function dt_training_request_custom_status_page($node, $status) {
  $response                                                        = array();
  $response['status']                                              = 1;
  $node->field_progress_custom_status[ LANGUAGE_NONE ][0]['value'] = $status;
  node_save($node);
  drupal_json_output($response);
  drupal_exit();
}

function ditoolsi_training_accept_request_page($course, $request) {
  global $user;

  $token = isset($_GET['token']) ? $_GET['token'] : '';

  if (!drupal_valid_token($token, "request-{$course->nid}-{$request->nid}-accept")) {
//    return MENU_ACCESS_DENIED;
  }

  db_insert('ditoolsi_training_pupils_list')->fields(array(
    'course_id'      => $course->nid,
    'user_id'        => $request->uid,
    'current_lesson' => 1,
    'created'        => time(),
    'status'         => DITOOLSI_TRAINING_PUPIL_ACTIVE,
  ))->execute();

  $request->field_request_course_status = array(
    LANGUAGE_NONE => array(
      array(
        'value' => DT_TRAINING_REQUEST_STATUS_APPROVED,
      ),
    ),
  );

  $query = db_select('node', 'n');
  $query->join('field_data_field_lesson_number', 'ln', 'ln.entity_id = n.nid');
  $query->join('field_data_field_lesson_course', 'lc', 'lc.entity_id = n.nid');
  $query->fields('n', array('nid'))
    ->condition('lc.field_lesson_course_target_id', $course->nid)
    ->range(0, 1)
    ->orderBy('ln.field_lesson_number_value');

  $nid          = $query->execute()->fetchField();
  $lesson       = node_load($nid);
  $account      = ditoolsi_profile_account($request);
  $name         = '<b>' . ditoolsi_profile_name($user) . '</b>';
  $course_title = l($course->title, "course/{$course->nid}");

  if ($lesson) {
    $lesson_title = l($lesson->title, "course/{$course->nid}/{$lesson->nid}");
    $text         = $name . ' открыл вам доступ к курсу ' . $course_title . '. Вам доступен первый урок ' . $lesson_title;
    ditoolsi_add_notification($account, $text);
  }

  node_save($request);
  drupal_set_message(t('The request is approved'));
  drupal_goto('im/requests');
}

function ditoolsi_training_reject_request_page($course, $request) {
  $token = isset($_GET['token']) ? $_GET['token'] : '';

  if (!drupal_valid_token($token, "request-{$course->nid}-{$request->nid}-reject")) {
//    return MENU_ACCESS_DENIED;
  }

  $request->field_request_course_status = array(
    LANGUAGE_NONE => array(
      array(
        'value' => DT_TRAINING_REQUEST_STATUS_REJECTED,
      ),
    ),
  );
  node_save($request);
  drupal_set_message(t('The request is rejected'), 'error');
  drupal_goto('im/requests');
}

function dt_training_ereg_check_page() {
  global $user;

  drupal_add_http_header('Access-Control-Allow-Origin', '*');

  if (user_is_logged_in()) {
    $response = array(
      'redirect' => url('user', array(
        'absolute' => TRUE,
      )),
    );

    drupal_json_output($response);
    drupal_exit();
  }

  $response = array();
  $valid    = TRUE;

  extract($_POST);

  if (empty($name)) {
    $response['error']['name'] = 'Поле обязательно для заполнения!';
    $valid                     = FALSE;
  }

  if (empty($mail) || !valid_email_address($mail)) {
    $response['error']['mail'] = 'E-mail не корректный!';
    $valid                     = FALSE;
  }

  if (empty($phone)) {
    $response['error']['phone'] = 'Поле обязательно для заполнения!';
    $valid                      = FALSE;
  }

  $nids = $uids = array();

  if (empty($course_id) || !is_array($course_id)) {
    $response['error']['course_id'] = 'invlid array of courses ids';
    $valid                          = FALSE;
  }
  else {
    foreach ($course_id as $nid) {
      if (!$node = node_load($nid)) {
        $response['error']['course_id'] = 'course ' . $nid . ' not exists';
        $valid                          = FALSE;
        break;
      }

      $nids[] = $node->nid;
      $uids[] = $node->uid;
    }
  }

  $parts = isset($vk) ? parse_url($vk) : array();

  if (!empty($vk) && !(isset($parts['host']) && ($parts['host'] == 'vk.com' || $parts['host'] == 'vkontakte.ru' || $parts['host'] == 'new.vk.com'))) {
    $response['error']['vk'] = 'Введите правильную ссылку на страницу Вконтакте';
    $valid                   = FALSE;
  }

  if (empty($token)) {
    $response['error']['token'] = 'token is required';
    $valid                      = FALSE;
  }

  if ($valid) {
    $uids = array_unique($uids);
    $hash = md5(json_encode($uids));

    if (md5(md5(md5('user-' . $hash . '-ereg'))) !== $token) {
      $response['error']['token'] = 'token is invalid';
    }
    elseif (user_load_by_mail($mail)) {
      $response['error']['mail'] = 'Пользователь с таким E-mail уже существует';
    }
    else {
      $roles  = user_roles();
      $fields = array(
        'name'             => $mail,
        'mail'             => $mail,
        'pass'             => DITOOLSI_DEFAULT_PASS,
        'status'           => 1,
        'init'             => $mail,
        'roles'            => array(DITOOLSI_PUPIL_RID => $roles[ DITOOLSI_PUPIL_RID ]),
        'access'           => REQUEST_TIME,
        'field_first_name' => array(
          LANGUAGE_NONE => array(
            0 => array(
              'value' => $name,
            ),
          ),
        ),
        'field_phone'      => array(
          LANGUAGE_NONE => array(
            0 => array(
              'value' => $phone,
            ),
          ),
        ),
      );

      user_save('', $fields);

      if ($uid = user_authenticate($mail, DITOOLSI_DEFAULT_PASS)) {
        $query = db_insert('ditoolsi_training_pupils_list')->fields(array(
          'course_id',
          'user_id',
          'current_lesson',
          'created',
          'status',
        ));

        $nid     = $nids[0];
        $node    = node_load($nid);
        $teacher = user_load($node->uid);

        if ($teacher->uid == 3433) {
          $data['token']   = 'iiS9XuWtDQ61o8VTMoaJpM8VjvUnk2Z0Cp1FkbEO5GKhK2zyHBoR06bgoOmwkprAIeze8tfCONSZtVDH8HbNJn';
          $data['subject'] = 'Центр финансовой культуры сообщает - Вам доступен курс.';
          $data['body']    = 'Заходите на личный кабинет по ссылке: http://fincult.ru/lichkab/<br>
Ваш логин: ' . $mail . '<br>
Ваш начальный пароль: 1<br>
<br>
ОБЯЗАТЕЛЬНО СМЕНИТЕ ПАРОЛЬ ПРИ ПЕРВОМ ВХОДЕ! <br>
<br>
Инструкция по работе с платформой обучения займет 7 мин (ссылкой будет, уроки работы самым непонятливым)<br>
<br>
С уважение, команда Центра финансовой культуры Романа Аргашокова<br>
Наш тел 8 (905) 222-62-86,<br>
Наш сайт fincult.ru';
          $data['to']      = $mail;
          file_get_contents('http://obuchenie.fincult.ru/mail.php?' . http_build_query($data));

          $data['subject'] = 'У Вас новый ученик - ' . variable_get('site_name', '');
          $data['body']    = '
  ************************************************************************

   ID Ученика: ' . $uid . '
   Имя: ' . $name . '
   E-mail: ' . $mail . '
   Телефон: ' . $phone . '

  ************************************************************************
  ';
          $data['to']      = $teacher->mail;
          file_get_contents('http://obuchenie.fincult.ru/mail.php?' . http_build_query($data));
        }
        else {
          ditoolsi_profile_send_mail_to_user($name, $mail);
          $from = variable_get('site_mail');
          drupal_mail('system', 'mail', $teacher->mail, language_default(), array(
            'context' => array(
              'subject' => 'У Вас новый ученик - ' . variable_get('site_name', ''),
              'message' => '
  ************************************************************************

   ID Ученика: ' . $uid . '
   Имя: ' . $name . '
   E-mail: ' . $mail . '
   Телефон: ' . $phone . '

  ************************************************************************
  ',
            ),
          ), $from);
        }


        foreach ($nids as $nid) {
          $query->values(array(
            'course_id'      => $nid,
            'user_id'        => $uid,
            'current_lesson' => 1,
            'created'        => REQUEST_TIME,
            'status'         => DITOOLSI_TRAINING_PUPIL_ACTIVE,
          ));
        }

        $query->execute();
        $response['redirect'] = url('ereg/signin', array(
          'absolute' => TRUE,
          'query'    => array(
            'id'    => $uid,
            'token' => md5(md5(md5('user-' . $uid . '-ereg-signin'))),
          ),
        ));
      }
      else {
        $response['error']['err'] = 'An error occurred';
      }
    }
  }

  drupal_json_output($response);
}

function dt_training_ereg_signin_page() {
  global $user;

  $id    = isset($_GET['id']) ? $_GET['id'] : -1;
  $token = isset($_GET['token']) ? $_GET['token'] : -2;

  if (md5(md5(md5('user-' . $id . '-ereg-signin'))) !== $token) {
    return MENU_ACCESS_DENIED;
  }
  elseif (!user_load($id)) {
    return MENU_NOT_FOUND;
  }

  $user = user_load($id);
  user_login_finalize();
  user_save($user);
  drupal_goto('courses');
}

function dt_training_esig_page() {
  drupal_add_http_header('Access-Control-Allow-Origin', '*');

  $name     = isset($_POST['name']) ? $_POST['name'] : '';
  $pass     = isset($_POST['pass']) ? $_POST['pass'] : '';
  $response = array();

  if ($uid = user_authenticate($name, $pass)) {
    $token                = md5(md5(md5('user-' . $uid . '-ereg-signin')));
    $response['redirect'] = url('ereg/signin', array(
      'query'    => array(
        'id'    => $uid,
        'token' => $token,
      ),
      'absolute' => TRUE,
    ));
  }
  else {
    $response['error'] = 'Неправильный логин или пароль';
  }

  drupal_json_output($response);
  drupal_exit();
}

function dt_training_epass_page() {
  global $language;

  drupal_add_http_header('Access-Control-Allow-Origin', '*');

  $name     = isset($_POST['name']) ? $_POST['name'] : '';
  $response = array();

  if ($account = user_load_by_name($name)) {
    $mail               = _user_mail_notify('password_reset', $account, $language);
    $response['notify'] = t('Further instructions have been sent to your e-mail address.');
    if (!empty($mail)) {
      watchdog('user', 'Password reset instructions mailed to %name at %email.', array(
        '%name'  => $account->name,
        '%email' => $account->mail,
      ));
    }
  }
  else {
    $response['error'] = 'Пользователя с таким логином не существует';
  }

  drupal_json_output($response);
  drupal_exit();
}

function dt_training_support_page($node) {
  $teacher_name = ditoolsi_profile_name($node->uid);

  $query = db_select('ditoolsi_training_curators_list', 'tcl');
  $query->condition('tcl.course_id', $node->nid);
  $query->fields('tcl', array('user_id'));
  $uids      = $query->execute()->fetchCol();
  $accounts  = user_load_multiple($uids);
  $courators = array_map(function ($account) {
    return l(ditoolsi_profile_name($account), "user/{$account->uid}") . '<br>';
  }, $accounts);

  $html = '<div class="tech-support">
  <div class="course-teacher">
    <label>' . t('Teacher') . ':</label>
    <div class="name">' . l($teacher_name, "user/{$node->uid}") . '</div>
  </div><br>
  ';

  if ($courators) {
    $html .= '<div class="course-courators">
    <label>' . t('Curator(s)') . ':</label>
    <div class="courators-content">
      ' . implode($courators) . '
    </div>
  </div><br>';
  }

  if ($node->uid == 3433) {
    return $html .= 'По вопросам технической поддержки, работы с платформой/обучающей системой:<br>
1. Пишите личным сообщением ВКонтакте: <a href="https://vk.com/id356297579" target="_blank">https://vk.com/id356297579</a> (Игорь Герасимов)<br>
2. Пишите на почту: <a href="mailto:support@fincult.ru">support@fincult.ru</a><br>
3. Звоните по телефону: +7-905-222-62-86<br>
4. Добавляйтесь в Skype: <a href="skype:fincultteh?chat">fincultteh</a> (Центр финансовой культуры. Тех.поддержка)';
  }


  return $html . '
  <div class="tech-support">
Тех. Поддержка (только по техническим вопросам, с 11-00 до 19-00 МСК). <br>
Ольга
      <div class="bottom skype">Skype: olga_zotova8</div>
      <div class="bottom mail">E-mail: 821olgazotova@gmail.com</div>
      <div class="bottom mail">Вк: https://vk.com/id239614759</div>
    </div>';
}

function ditoolsi_training_course_request_page($course) {
  global $user;

  $node                          = entity_create('node', array(
    'title'  => t('Request for learning [uid:!uid], [nid:!nid]', array(
      '!uid' => $user->uid,
      '!nid' => $course->nid,
    )),
    'type'   => 'request_course',
    'status' => NODE_PUBLISHED,
    'uid'    => $user->uid,
  ));
  $node->field_request_course_id = array(
    LANGUAGE_NONE => array(
      0 => array(
        'target_id' => $course->nid,
      ),
    ),
  );

  $account = ditoolsi_profile_account($user);
  $phone   = '<i>Не указан</i>';

  if (isset($account->field_phone[ LANGUAGE_NONE ][0]['value'])) {
    $phone = $account->field_phone[ LANGUAGE_NONE ][0]['value'];
  }

  $text = 'У вас новая заявка на курс: "' . l($course->title, "node/{$course->nid}") . '"

Клиент: 
Имя: ' . ditoolsi_profile_name($user) . '
Телефон: ' . $phone . '
E-mail: ' . $user->mail;

  drupal_mail('system', __FUNCTION__, ditoolsi_profile_account($course)->mail, language_default(),
    array(
      'context' => array(
        'subject' => 'Новая заявка на курс в Профологии',
        'message' => $text,
      ),
    ));

  node_save($node);
  drupal_goto('course/' . $course->nid . '/request-success');
}

function ditoolsi_training_course_request_success_page($course) {
  if (isset($course->field_course_success_request[ LANGUAGE_NONE ][0]['value'])) {
    $value  = $course->field_course_success_request[ LANGUAGE_NONE ][0]['value'];
    $format = $course->field_course_success_request[ LANGUAGE_NONE ][0]['format'];
    $text   = check_markup($value, $format);
    $text   = token_replace($text, array(
      'node' => $course,
    ));
  }
  else {
    $text = '<p>Благодарим, что отправили заявку на курс: "' . $course->title . '".</p>';
    $text .= '<p>Наши специалисты свяжутся с вами в ближайшее время!</p>';
  }

  return $text;
}

function dt_training_toggle_statistic_block($node) {
  $commands = array();

  if (!empty($node->field_course_visible_sb[ LANGUAGE_NONE ][0]['value'])) {
    $node->field_course_visible_sb = array();
    $commands[]                    = ajax_command_html('#toggle-statistic-block',
      t('Show block with statistics'));
    $commands[]                    = ajax_command_invoke('#statistic-block', 'empty');
  }
  else {
    ctools_include('ajax');

    $node->field_course_visible_sb[ LANGUAGE_NONE ][0]['value'] = 1;
    $commands[]                                                 = ajax_command_html('#toggle-statistic-block',
      t('Hide block with statistics'));
    $commands[]                                                 = ctools_ajax_command_reload();
  }

  node_save($node);

  return array(
    '#type'     => 'ajax',
    '#commands' => $commands,
  );
}

function dt_training_save_order_courses_page() {
  global $user;

  $order                           = isset($_POST['order']) && is_array($_POST['order']) ? $_POST['order'] : array();
  $account                         = ditoolsi_profile_account($user);
  $account->data['_order_courses'] = $order;
  user_save($account);

  drupal_json_output(array());
  drupal_exit();
}

function dt_training_start_learn2_page($node) {
  global $user;

  $query = db_select('ditoolsi_training_pupils_list', 'pl')
    ->fields('pl', array('current_lesson', 'status', 'started'))
    ->fields('lc', array('entity_id'));

  $query->join('node', 'n', 'n.nid = pl.course_id');
  $query->leftJoin('field_data_field_lesson_course', 'lc',
    'lc.field_lesson_course_target_id = n.nid');
  $query->leftJoin('field_data_field_lesson_number', 'ln',
    'ln.entity_id = lc.entity_id AND ln.field_lesson_number_value = pl.current_lesson');

  $pupil = $query->condition('pl.user_id', $user->uid)
    ->condition('pl.course_id', $node->nid)
    ->range(0, 1)
    ->execute()
    ->fetchObject();

  if (!$pupil || $pupil->started) {
    return MENU_ACCESS_DENIED;
  }

  if (!$pupil->current_lesson) {
    db_update('ditoolsi_training_pupils_list')->fields(array(
      'current_lesson' => 1,
    ))->condition('user_id', $user->uid)->condition('course_id', $node->nid)->execute();

    $pupil->current_lesson = 1;
  }

  if (!$pupil->entity_id) {
    $lesson = dt_training_load_lesson_by_number($node->nid, $pupil->current_lesson);
  }
  else {
    $lesson = node_load($pupil->entity_id);
  }

  if ($lesson) {
    dt_training_update_start_time_progress($lesson, $user);
    db_update('ditoolsi_training_pupils_list')
      ->fields(array('started' => REQUEST_TIME))
      ->condition('user_id', $user->uid)
      ->condition('course_id', $node->nid)
      ->execute();
    drupal_set_message('Вы можете приступить к обучению');
  }

  drupal_goto("course/{$node->nid}");
}

function mgc_pro_bibneva_course_register_page() {
  global $user;

  if (user_is_logged_in()) {
    drupal_goto('user');
  }

  extract($_REQUEST);
  //print_r($_GET);
  print_r($_REQUEST);
  if (empty($name)) {
    watchdog(__FUNCTION__, 'empty name');
    print 'empty name';

    return;
  }
  elseif (empty($mail) || !valid_email_address($mail)) {
    watchdog(__FUNCTION__, 'empty mail');
    print 'empty mail';

    return;
  }
  elseif (empty($phone)) {
    watchdog(__FUNCTION__, 'empty phone');
    print 'empty phone';

    return;
  }
  elseif (empty($course_id)) {
    watchdog(__FUNCTION__, 'empty course_id');
    print 'empty course_id';

    return;
  }
  elseif (!node_load($course_id)) {
    watchdog(__FUNCTION__, 'course not exists');
    print 'course not exists';

    return;
  }
  elseif (!defined('DT_TRAINING_BIBNEVA_COURSE_' . $course_id)) {
    watchdog(__FUNCTION__, 'invalid course_id' . 'DT_TRAINING_BIBNEVA_COURSE_' . $course_id);
    print 'invalid course_id';

    return;
  }

  $roles  = user_roles();
  $fields = array(
    'name'             => $mail,
    'mail'             => $mail,
    'pass'             => DITOOLSI_DEFAULT_PASS,
    'status'           => 1,
    'init'             => $mail,
    'roles'            => array(DITOOLSI_PUPIL_RID => $roles[ DITOOLSI_PUPIL_RID ]),
    'access'           => REQUEST_TIME,
    'field_first_name' => array(
      LANGUAGE_NONE => array(
        0 => array(
          'value' => $name,
        ),
      ),
    ),
    'field_phone'      => array(
      LANGUAGE_NONE => array(
        0 => array(
          'value' => $phone,
        ),
      ),
    ),
  );
  print_r($fields);
  $node    = node_load($course_id);
  $teacher = user_load($node->uid);
  echo 'a';
  user_save('', $fields);
  echo 'b';
  $lesson_id = reset($node->lessons)->nid;

  echo $uid = user_authenticate($mail, DITOOLSI_DEFAULT_PASS);
  if ($uid = user_authenticate($mail, DITOOLSI_DEFAULT_PASS)) {
    db_insert('ditoolsi_training_pupils_list')->fields(array(
      'course_id'      => $course_id,
      'user_id'        => $uid,
      'current_lesson' => 1,
      'created'        => REQUEST_TIME,
      'status'         => DITOOLSI_TRAINING_PUPIL_ACTIVE,
    ))->execute();

    $user = user_load($uid);
    user_login_finalize();
    $account       = user_save($user);
    $first_task_id = $node->lessons[ $lesson_id ]->tasks;
    $first_task_id = reset($first_task_id);
    //$stop = 'Stop';
    drupal_set_message(t('Поздравляем! Вам уже доступен для изучения курс "@course_title". <br>
Сейчас Вам нужно посмотреть первый видеоурок и выполнить !task.<br>
Мы желаем Вам успехов в освоении курса! Пора сделать первый шаг))', array(
      '@course_title' => $node->title,
      '!link'         => l(t('первый видеоурок'), 'course/' . $node->nid . '/' . $lesson_id),
      '!task'         => l(t('первые задания к уроку'), '', array(
        'external' => TRUE,
        'fragment' => 'to-task-0',
      )),

    )));
    //  drupal_set_message(t('Congratulations!!! You have successfully signed up on the site. '), 'status');
    $target = $find = '';

    //module_load_include('php', 'ditoolsi', 'classes/GetResponseAPI.class');
    ditoolsi_profile_send_mail_teacher($name, $mail, '', $target, $find);
    ditoolsi_training_send_mail_to_pupil($account, $node->nid);
    //$api      = new GetResponse('d02fb8f492a44b6186984071ac219a8d');
    // $campaign = $api->getCampaignByName('paf_promo_course');
//    $api->addContact($campaign, $name, $mail, 'standard', 0, array(
//      'telefon_text' => '',
//    ));
    mgc_start_learn2($node);
    drupal_goto('course/' . $course_id . '/' . $lesson_id);
  }
}

function mgc_pro_bibneva_course_check_page() {
  drupal_add_http_header('Access-Control-Allow-Origin', '*');

  $response = array();
  $valid    = TRUE;

  extract($_POST);

//  if (empty($name)) {
//    $response['error']['.form-item-name'] = 'Поле обязательно для заполнения!';
//    $valid                                = FALSE;
//  }
//
//  if (empty($mail) || !valid_email_address($mail)) {
//    $response['error']['.form-item-mail'] = 'E-mail не валидный!';
//    $valid                                = FALSE;
//  }
  $stop = 'Stop';
  /*Убираем, так как не нужен больше телефон*/
  /* if (empty($phone)) {
     $response['error']['.form-item-phone'] = 'Поле обязательно для заполнения!';
     $valid                                 = FALSE;
   }*/

//  if (empty($vk)) {
//    $response['error']['.form-item-vk'] = 'Поле обязательно для заполнения!';
//    $valid                              = FALSE;
//  }

  if (empty($course_id)) {
    watchdog(__FUNCTION__, 'empty course_id');
    print 'empty course_id';

    return;
  }

  if (!node_load($course_id)) {
    watchdog(__FUNCTION__, 'course not exists');
    print 'course not exists';

    return;
  }

  if (!defined('DT_TRAINING_BIBNEVA_COURSE_' . $course_id)) {
    watchdog(__FUNCTION__, 'invalid course_id');
    print 'invalid course_id';

    return;
  }

  if ($valid) {
    if (user_load_by_mail($mail)) {
//      $response['error']['.form-item-mail'] = t('User with this email already exists') . '<br>' .
//        t('If this is your account, you can enter the site by clicking on the link: !link',
//          array(
//            '!link' => l(url('user/login', array(
//              'absolute' => TRUE,
//            )), 'user/login', array(
//              'absolute'   => TRUE,
//              'attributes' => array(
//                'target' => '_blank',
//              ),
//            )),
//          ));
    }
    else {
      if (isset($_GET['q'])) {
        unset($_GET['q']);
      }

      $response['redirect'] = url('pro-bibneva/register', array(
        'absolute' => TRUE,
        'query'    => $_POST,
      ));
    }
  }

  drupal_json_output($response);
}
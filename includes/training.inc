<?php

function ditoolsi_training_get_tasks($lesson_id) {
  $query = new EntityFieldQuery();

  $query->entityCondition('entity_type', 'node')
    ->fieldCondition('field_task_lesson', 'target_id', $lesson_id);

  $result = $query->execute();

  if (isset($result['node'])) {
    $nids = array_keys($result['node']);

    return node_load_multiple($nids);
  }
  else {
    return array();
  }
}

function ditoolsi_training_count_lessons($course_id) {
  $cache = &drupal_static(__FUNCTION__);

  if (isset($cache[$course_id])) {
    return $cache[$course_id];
  }

  $course = node_load($course_id);

  $query = db_select('node', 'lesson');
  $query->join('field_data_field_lesson_course', 'lesson_course',
    'lesson_course.entity_id = lesson.nid');
  $query->addExpression('COUNT(lesson.nid)', 'lessons');
  $query->condition('lesson.type', 'lesson')
    ->condition('lesson.uid', $course->uid)
    ->condition('lesson_course.field_lesson_course_target_id', $course_id);

  $cache[$course_id] = $query->execute()->fetchField();

  return $cache[$course_id];
}

function ditoolsi_training_get_lessons(
  $course,
  $current_lesson,
  $account = NULL
) {
  global $user;

  if (empty($account)) {
    $account = $user;
  }

  $visible = DT_TRAINING_LESSON_VISIBLE_VISIBLE_ALL_ACCESS_ONE;

  if ($current_lesson > 0) {
    $query = db_select('node', 'lesson');
    $query->join('field_data_field_lesson_course', 'lesson_course',
      'lesson_course.entity_id = lesson.nid');
    $query->join('field_data_field_lesson_number', 'lesson_number',
      'lesson_number.entity_id = lesson.nid');
    $query->addField('lesson', 'nid');
    $query->condition('lesson.type', 'lesson')
      ->condition('lesson_course.field_lesson_course_target_id', $course->nid);

    if (isset($course->field_course_lessons_visible[LANGUAGE_NONE][0]['value'])) {
      $visible = $course->field_course_lessons_visible[LANGUAGE_NONE][0]['value'];
    }

    switch ($visible) {
      case DT_TRAINING_LESSON_VISIBLE_VISIBLE_ONE_ACCESS_ONE:
        $query->condition('lesson_number.field_lesson_number_value',
          $current_lesson, '<=');
        break;

      case DT_TRAINING_LESSON_VISIBLE_VISIBLE_MANUAL_ACCESS_MANUAL:
        $wrapper = entity_metadata_wrapper('node', $course);
        $ids     = $wrapper->field_course_lessons_list->raw();
        $or      = db_or();

        if ($ids) {
          $or->condition('lesson.nid', $ids, 'IN');
        }
        else {
          $or->condition('lesson.nid', -1);
        }

        $or->condition('lesson_number.field_lesson_number_value',
          $current_lesson, '<=');
        $query->condition($or);
        break;
    }

    $query->orderBy('lesson_number.field_lesson_number_value');

    $lessons_id = $query->execute()->fetchCol();
    $lessons    = node_load_multiple($lessons_id);
  }
  else {
    $lessons    = $course->lessons;
    $lessons_id = array_keys($lessons);
  }

  if (count($lessons_id)) {
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'node')
      ->entityCondition('bundle', 'lesson_progress')
      ->propertyCondition('uid', $account->uid)
      ->fieldCondition('field_progress_lesson_id', 'target_id', $lessons_id,
        'IN');
    $items = $query->execute();

    if (isset($items['node'])) {
      $progresses_id   = array_keys($items['node']);
      $progresses_node = node_load_multiple($progresses_id);

      foreach ($progresses_node as $progress) {
        $p_wrapper = entity_metadata_wrapper('node', $progress);
        $lesson_id = $p_wrapper->field_progress_lesson_id->raw();
        if (isset($lessons[$lesson_id])) {
          $lessons[$lesson_id]->progress = $p_wrapper;
        }
      }
    }
  }

  uasort($lessons, function ($a, $b) {
    return ($a->field_lesson_number[LANGUAGE_NONE][0]['value'] < $b->field_lesson_number[LANGUAGE_NONE][0]['value']) ? -1 : 1;
  });

  return $lessons;
}

function ditoolsi_training_get_courses_nids() {
  $query  = ditoolsi_training_get_cources_query();
  $result = $query->execute()->fetchAllKeyed(0, 1);
  $nids   = array_keys($result);

  return $nids;
}

function ditoolsi_training_get_courses() {
  $nids      = ditoolsi_training_get_courses_nids();
  $all_nodes = node_load_multiple($nids);

  return $all_nodes;
}


function ditoolsi_training_block_autotest($task, $account) {
  $query = db_select('node', 'n');
  $query->join('field_data_field_progress_lesson_id', 'lesson_id',
    'lesson_id.entity_id = n.nid');
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
  $query->join('field_data_field_autotest_time_completion', 'time_completion',
    'time_completion.entity_id = node_task.nid');
  $query->join('field_data_field_progress_start_autotest', 'start_autotest',
    'start_autotest.entity_id = node_progress_task.nid');

  $query->fields('node_progress_task', array('nid'))
    ->condition('n.type', 'lesson_progress')
    ->condition('node_progress_task.type', 'auto_test_progress')
    ->condition('node_task.type', 'lesson_auto_test')
    ->condition('node_task.nid', $task->nid)
    ->condition('n.uid', $account->uid)
    ->condition('node_progress_task.uid', $account->uid);

  $result = $query->execute()->fetchObject();

  if ($result) {
    $node                                                      = node_load($result->nid);
    $node->field_progress_blocked[LANGUAGE_NONE][0]['value']   = 1;
    $node->field_progress_time_lock[LANGUAGE_NONE][0]['value'] = REQUEST_TIME;
    node_save($node);

    return $node;
  }
}

function ditoolsi_training_finish_autotest($task, $account) {
  $query = db_select('node', 'n');
  $query->join('field_data_field_progress_tasks', 'progress_tasks',
    'progress_tasks.entity_id = n.nid');
  $query->join('field_data_field_progress_task_progress', 'task_progress',
    'task_progress.entity_id = progress_tasks.field_progress_tasks_value');
  $query->join('node', 'node_progress_task',
    'node_progress_task.nid = task_progress.field_progress_task_progress_target_id');
  $query->join('field_data_field_progress_task', 'progress_task',
    'progress_task.entity_id = node_progress_task.nid');

  $query->condition('progress_task.field_progress_task_target_id', $task->nid)
    ->condition('node_progress_task.uid', $account->uid)
    ->fields('node_progress_task', array('nid'))
    ->fields('progress_tasks', array('field_progress_tasks_value'));

  $item = $query->execute()->fetchObject();

  if ($item) {
    $fc_item    = field_collection_item_load($item->field_progress_tasks_value);
    $node       = node_load($item->nid);
    $fc_wrapper = entity_metadata_wrapper('field_collection_item', $fc_item);
    $fc_wrapper->field_progress_status->set(DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED);
    $fc_wrapper->save();
  }
}

function ditoolsi_training_is_lesson_finished($lesson, $account) {

  $account = ditoolsi_profile_account($account, FALSE);
  if (!$account) {
    return FALSE;
  }

  $statuses = array(
    DITOOLSI_TRAINING_PROGRESS_STATUS_NEW,
    DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST,
    DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW,
    DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED,
  );

  $query = db_select('node', 'n');
  $query->join('field_data_field_progress_tasks', 'progress_tasks',
    'progress_tasks.entity_id = n.nid');
  $query->join('field_data_field_progress_task_progress',
    'progress_task_progress',
    'progress_task_progress.entity_id = progress_tasks.field_progress_tasks_value');
  $query->join('field_data_field_progress_status', 'progress_status',
    'progress_status.entity_id = progress_tasks.field_progress_tasks_value');
  $query->join('field_data_field_progress_task', 'progres_task',
    'progres_task.entity_id = progress_task_progress.field_progress_task_progress_target_id');

  $nids = array_keys($lesson->tasks);

  if ($nids) {
    $query->condition('n.uid', $account->uid)
      ->condition('progres_task.field_progress_task_target_id', $nids, 'IN')
      ->condition('progress_status.field_progress_status_value', $statuses,
        'IN');

    $query->addField('progress_status', 'field_progress_status_value',
      'status');
    $result = $query->execute()->fetchCol();
  }
  else {
    $result = array();
  }

  $finished  = 0;
  $to_review = 0;

  foreach ($result as $col) {
    switch ($col) {
      case DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED:
        $finished++;
        break;

      case DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW:
        $to_review++;
        break;
    }
  }

  return ($to_review && ($finished + $to_review) == count($lesson->tasks));
}

function ditoolsi_training_is_lesson_recast($lesson, $account) {
  $account = ditoolsi_profile_account($account, FALSE);
  if (!$account) {
    return FALSE;
  }

  $query = db_select('node', 'n');
  $query->join('field_data_field_progress_tasks', 'progress_tasks',
    'progress_tasks.entity_id = n.nid');
  $query->join('field_data_field_progress_task_progress',
    'progress_task_progress',
    'progress_task_progress.entity_id = progress_tasks.field_progress_tasks_value');
  $query->join('field_data_field_progress_status', 'progress_status',
    'progress_status.entity_id = progress_tasks.field_progress_tasks_value');
  $query->join('field_data_field_progress_task', 'progres_task',
    'progres_task.entity_id = progress_task_progress.field_progress_task_progress_target_id');

  $nids = array_keys($lesson->tasks);

  $query->condition('n.uid', $account->uid)
    ->condition('progres_task.field_progress_task_target_id', $nids, 'IN')
    ->condition('progress_status.field_progress_status_value',
      DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST);

  return $query->countQuery()->execute()->fetchField();
}

function ditoolsi_training_is_lesson_all_finished($lesson, $account) {
  $account = ditoolsi_profile_account($account, FALSE);
  if (!$account) {
    return FALSE;
  }


  $query = db_select('node', 'n');
  $query->join('field_data_field_progress_tasks', 'progress_tasks',
    'progress_tasks.entity_id = n.nid');
  $query->join('field_data_field_progress_task_progress',
    'progress_task_progress',
    'progress_task_progress.entity_id = progress_tasks.field_progress_tasks_value');
  $query->join('field_data_field_progress_status', 'progress_status',
    'progress_status.entity_id = progress_tasks.field_progress_tasks_value');
  $query->join('field_data_field_progress_task', 'progres_task',
    'progres_task.entity_id = progress_task_progress.field_progress_task_progress_target_id');

  $nids = array_keys($lesson->tasks);

  $query->condition('n.uid', $account->uid)
    ->condition('progres_task.field_progress_task_target_id', $nids, 'IN')
    ->condition('progress_status.field_progress_status_value',
      DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED);

  $count = $query->countQuery()->execute()->fetchField();

  return $count == count($lesson->tasks);
}

function ditoolsi_training_get_lesson_progress($lesson, $account) {
  $account = ditoolsi_profile_account($account, FALSE);

  if (!$account) {
    return FALSE;
  }

  $query = db_select('node', 'n');
  $query->join('field_data_field_progress_lesson_id', 'lesson_id',
    'lesson_id.entity_id = n.nid');
  $query->join('field_data_field_progress_status', 'status',
    'status.entity_id = n.nid');
  $query->condition('lesson_id.field_progress_lesson_id_target_id',
    $lesson->nid)->condition('n.uid', $account->uid)->fields('n', array('nid'));

  $nid = $query->execute()->fetchField();

  if (!$nid) {
    return FALSE;
  }

  $node = node_load($nid);

  if ($field_progress_status = field_get_items('node', $node,
    'field_progress_status')
  ) {
    $status = $field_progress_status[0]['value'];
  }
  else {
    $status = DITOOLSI_TRAINING_PROGRESS_STATUS_NEW;
  }

  $progress = array(
    'node'   => $node,
    'status' => $status,
    'tasks'  => array(),
  );

  $n_wrapper = entity_metadata_wrapper('node', $node);

  foreach ($n_wrapper->field_progress_tasks->value() as $fc_progress_task) {
    // Get status of task.
    if ($field_progress_status = field_get_items('field_collection_item',
      $fc_progress_task, 'field_progress_status')
    ) {
      $task_status = $field_progress_status[0]['value'];
    }
    else {
      $task_status = DITOOLSI_TRAINING_PROGRESS_STATUS_NEW;
    }

    // Get node of progress of task.
    if ($field_progress_task_progress = field_get_items('field_collection_item',
      $fc_progress_task, 'field_progress_task_progress')
    ) {
      $task_progress = node_load($field_progress_task_progress[0]['target_id']);
    }
    else {
      $task_progress = entity_create('node', array(
        'type' => 'text_task_progress',
        'uid'  => $user->uid,
      ));
    }

    // Get id of task.
    if ($field_progress_task = field_get_items('node', $task_progress,
      'field_progress_task')
    ) {
      $task_id                     = $field_progress_task[0]['target_id'];
      $progress['tasks'][$task_id] = array(
        'status' => $task_status,
        'node'   => $task_progress,
      );
    }
    else {
      $task_id;
    }
  }

  return $progress;
}

function ditoolsi_training_lesson_allowed_for_review($lesson, $account) {
  $progress = ditoolsi_training_get_lesson_progress($lesson, $account);

  if (!$progress) {
    return FALSE;
  }

  $allowed_statuses = array(
    'auto_test_progress' => array(
      DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED,
    ),
    'text_task_progress' => array(
      DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW,
      DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED,
    ),
    'lesson_progress'    => array(
      DITOOLSI_TRAINING_PROGRESS_STATUS_NEW,
      DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW,
      DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST,
    ),
  );

  $all     = count($lesson->tasks);
  $allowed = 0;

  foreach ($progress['tasks'] as $item) {
    if (in_array($item['status'], $allowed_statuses[$item['node']->type])) {
      $allowed++;
    }
  }

  $lesson_status = $progress['node']->field_progress_status[LANGUAGE_NONE][0]['value'];

  return $allowed >= $all && in_array($lesson_status,
      $allowed_statuses['lesson_progress']);
}

function ditoolsi_training_get_lesson_status($lesson, $account) {
  $account = ditoolsi_profile_account($account, FALSE);

  if (!$account) {
    return FALSE;
  }

  $query = db_select('node', 'n');
  $query->join('field_data_field_progress_lesson_id', 'lesson_id',
    'lesson_id.entity_id = n.nid');
  $query->join('field_data_field_progress_status', 'status',
    'status.entity_id = n.nid');
  $query->condition('lesson_id.field_progress_lesson_id_target_id',
    $lesson->nid)->condition('n.uid', $account->uid)->fields('n', array('nid'));

  $nid = $query->execute()->fetchField();

  if (!$nid) {
    return FALSE;
  }

  $node = node_load($nid);

  if ($field_progress_status = field_get_items('node', $node,
    'field_progress_status')
  ) {
    $status = $field_progress_status[0]['value'];
  }
  else {
    $status = DITOOLSI_TRAINING_PROGRESS_STATUS_NEW;
  }

  return $status;
}

function ditoolsi_training_is_lesson_complete($lesson, $account) {
  $finished = FALSE;

  $account = ditoolsi_profile_account($account, FALSE);

  if (!$account) {
    return FALSE;
  }

  $status = ditoolsi_training_get_lesson_status($lesson, $account);

  if ($status == DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED) {
    $finished = TRUE;
  }

  return $finished;
}

function ditoolsi_training_get_task_progress($task, $account) {
  $account = ditoolsi_profile_account($account, FALSE);

  if (!$account) {
    return FALSE;
  }

  $progress_status = DITOOLSI_TRAINING_PROGRESS_STATUS_NEW;

  $nid = $task->nid;

  $query = db_select('node', 'n');
  $query->join('field_data_field_progress_tasks', 'progress_tasks',
    'progress_tasks.entity_id = n.nid');
  $query->join('field_data_field_progress_task_progress',
    'progress_task_progress',
    'progress_task_progress.entity_id = progress_tasks.field_progress_tasks_value');
  $query->join('field_data_field_progress_status', 'progress_status',
    'progress_status.entity_id = progress_tasks.field_progress_tasks_value');
  $query->join('field_data_field_progress_task', 'progres_task',
    'progres_task.entity_id = progress_task_progress.field_progress_task_progress_target_id');
  $query->condition('n.uid', $account->uid)
    ->condition('progres_task.field_progress_task_target_id', $nid);
  $query->addField('progress_status', 'field_progress_status_value', 'status');
  $result = $query->execute()->fetchField();

  if ($result) {
    $progress_status = $result;
  }

  return $progress_status;
}

function ditoolsi_training_show_task_answer($task, $account) {
  $show_answer = FALSE;
  $account     = ditoolsi_profile_account($account, FALSE);

  if (!$account) {
    return FALSE;
  }

  $task_progress = ditoolsi_training_get_task_progress($task, $account);
  if ($task_progress == DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED || $task_progress == DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW) {
    $show_answer = TRUE;
  }

  return $show_answer;
}

function ditoolsi_training_get_task_status_class($status) {
  $status_class = '';
  switch ($status) {
    case DITOOLSI_TRAINING_PROGRESS_STATUS_NEW:
      $status_class = 'status-new';
      break;

    case DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED:
      $status_class = 'status-finished';
      break;

    case DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST:
      $status_class = 'status-recast';
      break;

    case DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW:
      $status_class = 'status-review';
      break;

    case DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW:
      $status_class = 'status-to-review';
      break;
  }

  return $status_class;
}

function ditoolsi_training_get_section($library_id) {
  $cache = &drupal_static(__FUNCTION__);
  if (isset($cache[$library_id])) {
    return $cache[$library_id];
  }

  $library = node_load($library_id);

  $query = db_select('node', 'section');
  $query->join('field_data_field_section_library', 'field_section_library',
    'field_section_library.entity_id = section.nid');
  $query->addField('section', 'nid');
  $query->condition('section.type', 'section')
    ->condition('section.uid', $library->uid)
    ->condition('field_section_library.field_section_library_target_id',
      $library_id);

  $nids = $query->execute()->fetchCol();

  $cache[$library_id] = node_load_multiple($nids);

  return $cache[$library_id];
}

function ditoolsi_training_get_libraries() {
  $nids  = array();
  $query = ditoolsi_training_get_libraries_query();
  if ($query) {
    $result = $query->execute()->fetchAllKeyed(0, 1);
    $nids   = array_keys($result);
  }
  $all_nodes = node_load_multiple($nids);

  return $all_nodes;
}

function ditoolsi_training_lesson_task_count($lesson) {
  return count($lesson->tasks);
}

function ditoolsi_training_lesson_video_count($lesson) {
  $wrapper = entity_metadata_wrapper('node', $lesson);

  $videos          = $wrapper->field_lesson_video->value();
  $videos_quantity = 0;

  foreach ($videos as $video) {
    $v_wrapper = entity_metadata_wrapper('field_collection_item', $video);

    switch ($v_wrapper->field_lesson_video_type->value()) {
      case DITOOLSI_TRAINING_VIDEO_TYPE_LOCAL:
        if ($v_wrapper->field_lesson_video_file->raw()) {
          $videos_quantity++;
        }
        break;

      case DITOOLSI_TRAINING_VIDEO_TYPE_YOUTUBE:
        if ($v_wrapper->field_lesson_video_url->raw()) {
          $videos_quantity++;
        }
        break;
    }
  }

  return $videos_quantity;
}

function ditoolsi_training_lesson_presentation_count($lesson) {
  $presentations     = field_get_items('node', $lesson,
    'field_lesson_presentation');
  $presentations_ids = array();

  foreach ($presentations as $presentation) {
    $presentation_ids[] = $presentation['value'];
  }
  $field_collection_presentations = field_collection_item_load_multiple($presentation_ids);
  $presentation_files_count       = 0;
  foreach ($field_collection_presentations as $field_collection_presentation) {
    if ($presentation_files = field_get_items('field_collection_item',
      $field_collection_presentation, 'field_lesson_presentation_file')
    ) {
      $presentation_files_count += count($presentation_files);
    }
  }

  return $presentation_files_count;
}

function ditoolsi_training_lesson_attach_info($lesson) {
  $info                  = array();
  $tasks_quantity        = ditoolsi_training_lesson_task_count($lesson);
  $video_quantity        = ditoolsi_training_lesson_video_count($lesson);
  $presentation_quantity = ditoolsi_training_lesson_presentation_count($lesson);

  $context = ditoolsi_prular($tasks_quantity);

  if ($tasks_quantity > 1) {
    $info[] = t('!count tasks', array(
      '!count' => $tasks_quantity,
    ), array(
      'context' => $context,
    ));
  }
  elseif ($tasks_quantity == 0 || $tasks_quantity == 1) {
    $info[] = t($tasks_quantity . ' task');
  }

  if ($video_quantity > 1) {
    $info[] = t('!count videos', array(
      '!count' => $video_quantity,
    ));
  }
  elseif ($video_quantity == 0 || $video_quantity == 1) {
    $info[] = t('!count video', array(
      '!count' => $video_quantity,
    ));
  }

  if ($presentation_quantity > 1) {
    $info[] = t('!count presentations', array(
      '!count' => $presentation_quantity,
    ));
  }
  elseif ($presentation_quantity == 0 || $presentation_quantity == 1) {
    $info[] = t('!count presentation', array(
      '!count' => $presentation_quantity,
    ));
  }

  return implode(' / ', $info);
}

function ditoolsi_training_lesson_status_view($lesson, $allow_view = TRUE) {
  global $user;

  $lesson_status = '';

  $query  = ditoolsi_training_get_cources_query();
  $result = $query->execute()->fetchCol();
  $course = node_load($lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id']);

  if (!in_array($lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'],
      $result) || !ditoolsi_training_access('pass lesson', $course)
  ) {
    return '';
  }
  elseif (!$allow_view) {
    return '<div class="lesson-status red">' . t('Closed', array(),
        array('context' => 'lesson')) . '</div>';
  }
  static $count_l = 0;
  if (!empty($lesson->progress)) {
    $status = $lesson->progress->field_progress_status->raw();

    switch ($status) {

      case DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED:
        $count_l++;
        $lesson_status = '<div class="lesson-status green">' . t('Complete') . '</div>';
        break;

      case DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST:
        $lesson_status = '<div class="lesson-status red">' . t('On completion') . '</div>';
        break;

      case DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW:
        $lesson_status = '<div class="lesson-status yellow">' . t('On review') . '</div>';
        break;

      case DITOOLSI_TRAINING_PROGRESS_STATUS_NEW:
      default:
        $lesson_status = '<div class="lesson-status yellow">' . t('In work') . '</div>';
        break;
    }
  }
  elseif (isset($user->roles[DITOOLSI_PUPIL_RID]) || $lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'] == DITOOLSI_TRAINING_TEACHER_COURSE) {
    $lesson_status = '<div class="lesson-status yellow">' . t('In work') . '</div>';
  }

  return $lesson_status;
}

function ditoolsi_training_get_current_lesson($course, $account = NULL) {
  global $user;

  $history = TRUE;

  if (empty($account)) {
    $account = $user;
    $history = FALSE;
  }

  $pupil = db_select('ditoolsi_training_pupils_list', 'pl')
    ->fields('pl', array('current_lesson', 'status'))
    ->condition('pl.user_id', $account->uid)
    ->condition('pl.course_id', $course->nid)
    ->execute()
    ->fetchObject();

  if ($pupil && ($history || $course->nid == MGC_TEACH_COURSE_ID)) {
    $current_lesson = $pupil->current_lesson;
  }
  elseif (ditoolsi_training_access('pass lesson', $course)) {
    if (isset($account->roles[DITOOLSI_CURATOR_RID]) || isset($account->roles[DITOOLSI_TEACHER_RID])) {
      $current_lesson = -1;
    }
    elseif ($pupil) {
      $current_lesson = $pupil->current_lesson;
    }
    else {
      $current_lesson = -2;
    }
  }
  else {
    $current_lesson = -2;
  }

  return $current_lesson;
}

function ditoolsi_training_get_first_lesson($course) {
  $nids = array_keys($course->lessons);

  if (!$nids) {
    return -1;
  }

  $query = db_select('node', 'lesson');
  $query->join('field_data_field_lesson_number', 'lesson_number',
    'lesson_number.entity_id = lesson.nid');
  $query->addField('lesson', 'nid');
  $query->condition('lesson.type', 'lesson')
    ->condition('lesson.nid', $nids, 'IN')
    ->orderBy('lesson_number.field_lesson_number_value');

  $lessons_id = $query->execute()->fetchField();

  return $lessons_id;
}

function ditoolsi_training_is_user_blocked($course, $account = NULL) {
  global $user;

  $blocked = FALSE;

  if (empty($account)) {
    $account = $user;
  }

  $query = db_select('ditoolsi_training_pupils_list', 'pl');
  $query->addField('pl', 'user_id');
  $query->condition('pl.user_id', $account->uid)
    ->condition('pl.course_id', $course->nid)
    ->condition('pl.status', DITOOLSI_TRAINING_PUPIL_BLOCKED);

  $result = $query->execute()->fetchField();

  if (!empty($result)) {
    $blocked = TRUE;
  }

  return $blocked;
}

function ditoolsi_training_add_pupil_to_course(
  $mail,
  $name,
  $phone,
  $course_id,
  $lesson
) {
  if (!empty($mail)) {
    if (valid_email_address($mail)) {
      $account = user_load_by_mail($mail);
      if (!empty($account)) {
        $query = db_select('ditoolsi_training_pupils_list',
          'training_pupils_list');
        $query->fields('training_pupils_list', array('id'));
        $query->condition('training_pupils_list.user_id', $account->uid);
        $query->condition('training_pupils_list.course_id', $course_id);
        $results = $query->execute()->fetchAll();
        if (!empty($results)) {
          drupal_set_message(t('Pupil @user is already on your course',
            array('@user' => $account->mail)), 'error');
        }
        else {
          db_insert('ditoolsi_training_pupils_list')->fields(array(
            'course_id'      => $course_id,
            'user_id'        => $account->uid,
            'current_lesson' => $lesson,
            'created'        => time(),
            'status'         => DITOOLSI_TRAINING_PUPIL_ACTIVE,
          ))->execute();
          drupal_set_message(t('Pupil @user successfully added to your course',
            array('@user' => $account->mail)));
        }
      }
      else {
        $roles                     = array();
        $roles[DITOOLSI_PUPIL_RID] = DITOOLSI_PUPIL_RID;
        $fields                    = array(
          'name'             => $mail,
          'mail'             => $mail,
          'pass'             => DITOOLSI_DEFAULT_PASS_USER,
          'status'           => 1,
          'init'             => $mail,
          'access'           => time(),
          'roles'            => $roles,
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

        $account = user_save('', $fields);
        $account = user_load_by_mail($mail); // Loading account

        ditoolsi_training_send_mail_to_pupil($account, $course_id);
        $add_user = db_insert('ditoolsi_training_pupils_list')->fields(array(
          'course_id'      => $course_id,
          'user_id'        => $account->uid,
          'current_lesson' => $lesson,
          'created'        => time(),
          'status'         => DITOOLSI_TRAINING_PUPIL_ACTIVE,
        ))->execute();
        drupal_set_message(t('Pupil @user successfully added to your course',
          array('@user' => $account->mail)));
      }
    }
    else {
      drupal_set_message(t('Wrong e-mail format @email',
        array('@email' => $mail)), 'error');
    }
  }
  else {
    drupal_set_message(t('Field e-mail can not be empty'), 'error');
  }
}

/**
 * Returns array with mail activation subject and body.
 *
 * @param object $node
 *   Node type of "course".
 *
 * @return array
 */
function ditoolsi_training_get_mail_activation($node = NULL) {
  $data = array(
    //    array('subject' => '', 'body' => ''),
    array('subject' => '', 'body' => ''),
    array('subject' => '', 'body' => ''),
  );

  if (isset($node->field_course_mail_subject[LANGUAGE_NONE][0]['value'])) {
    $data[0]['subject'] = $node->field_course_mail_subject[LANGUAGE_NONE][0]['value'];
  }

  if (isset($node->field_course_mail_body[LANGUAGE_NONE][0]['value'])) {
    $data[0]['body'] = $node->field_course_mail_body[LANGUAGE_NONE][0]['value'];
  }

  if (isset($node->field_course_mail_adding_subject[LANGUAGE_NONE][0]['value'])) {
    $data[1]['subject'] = $node->field_course_mail_adding_subject[LANGUAGE_NONE][0]['value'];
  }

  if (isset($node->field_course_mail_adding_body[LANGUAGE_NONE][0]['value'])) {
    $data[1]['body'] = $node->field_course_mail_adding_body[LANGUAGE_NONE][0]['value'];
  }

  return $data;
}

function ditoolsi_training_requests_review_query($course_id = NULL) {
  $query = db_select('node', 'n');
  $query->join('field_data_field_progress_lesson_id', 'lesson_id',
    'lesson_id.entity_id = n.nid');
  $query->join('users', 'u', 'u.uid = n.uid');
  $query->join('field_data_field_lesson_number', 'lesson_number',
    'lesson_number.entity_id = lesson_id.field_progress_lesson_id_target_id');
  $query->join('field_data_field_lesson_course', 'course',
    'course.entity_id = lesson_id.field_progress_lesson_id_target_id');
  $query->join('node', 'node_course',
    'node_course.nid = course.field_lesson_course_target_id');
  $query->join('field_data_field_progress_status', 'progress_status',
    'progress_status.entity_id = n.nid and progress_status.bundle = :bundle',
    array(':bundle' => 'lesson_progress'));
  $query->condition('progress_status.field_progress_status_value',
    DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW)
    ->fields('n', array('nid'))
    ->orderBy('n.changed', 'desc')
    ->orderBy('lesson_number.field_lesson_number_value', 'ASC');


  if ($course_id) {
    if (is_array($course_id)) {
      $query->condition('node_course.nid', $course_id, 'IN');
    }
    else {
      $query->condition('node_course.nid', $course_id);
    }
  }

  return $query;
}

function ditoolsi_training_request_sent($course) {
  static $result;
  global $user;


  if (!$result) {
    $pupil = db_select('ditoolsi_training_pupils_list', 'pl')
      ->condition('pl.user_id', $user->uid)
      ->condition('pl.course_id', $course->nid)
      ->countQuery()
      ->execute()
      ->fetchField();

    $query = db_select('node', 'n');
    $query->join('field_data_field_request_course_id', 'ci',
      'ci.entity_id = n.nid');
    $query->join('field_data_field_request_course_status', 's',
      's.entity_id = n.nid');
    $query->condition('n.uid', $user->uid)
      ->condition('ci.field_request_course_id_target_id', $course->nid)
      ->condition('s.field_request_course_status_value',
        DT_TRAINING_REQUEST_STATUS_NEW);
    $result = $query->countQuery()->execute()->fetchField() && !((bool) $pupil);
  }

  return $result;
}

function mgc_teach_course_lesson_status_view($lesson, $allow_view = TRUE) {
  global $user;

  $lesson_status = '';

  if (!mgc_user_has_teacher_role()) {
    return '';
  }
  elseif (!$allow_view) {
    return '<div class="lesson-status red">' . t('Closed', array(),
        array('context' => 'lesson')) . '</div>';
  }
  static $count_l = 0;
  if (!empty($lesson->progress)) {
    $status = $lesson->progress->field_progress_status->raw();

    switch ($status) {

      case DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED:
        $count_l++;
        $lesson_status = '<div class="lesson-status green">' . t('Complete') . '</div>';
        break;

      case DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST:
        $lesson_status = '<div class="lesson-status red">' . t('On completion') . '</div>';
        break;

      case DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW:
        $lesson_status = '<div class="lesson-status yellow">' . t('On review') . '</div>';
        break;

      case DITOOLSI_TRAINING_PROGRESS_STATUS_NEW:
      default:
        $lesson_status = '<div class="lesson-status yellow">' . t('In work') . '</div>';
        break;
    }
  }
  elseif (isset($user->roles[DITOOLSI_PUPIL_RID]) || $lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'] == MGC_TEACH_COURSE_ID) {
    $lesson_status = '<div class="lesson-status yellow">' . t('In work') . '</div>';
  }

  return $lesson_status;
}

function mgc_get_courses($user) {
  $nids = FALSE;
  if (user_is_logged_in()) {
    $query = ditoolsi_training_get_cources_query($user->uid, TRUE);
    $nids  = $query->execute()->fetchCol();
  }
  return $nids;
}

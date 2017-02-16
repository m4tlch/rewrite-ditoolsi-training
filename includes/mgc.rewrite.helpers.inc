<?php
function mgc_print_pupil_stats_table_form($form, &$form_state) {
  $course                          = $form_state['build_info']['args'][0];
  $form_state['storage']['course'] = $course;
  if (isset($form_state['storage']['course_stats'])) {
    $course_stats = $form_state['storage']['course_stats'];
  }
  else {
    $course_stats = mgc_get_course_stats($course);
  }
  $stats_by_uid_html = mgc_print_course_stats_by_uid($course, $course_stats);

  $form['stats_by_uid'] = array(
    '#markup' => $stats_by_uid_html,
  );


  $header         = array(
    'Date'       => t('Дата'),
    'Autochecks' => t('Количество автопроверок в день'),
    'Checks'     => t('Количество проверок в день'),
    'Recasts'    => t('Количество исправлений в день'),
    'Tendency'   => t('Тенденция'),

  );
  $rows           = array();
  $activity       = 0;
  $total_activity = 0;
  foreach ($course_stats as $key => $stat) {
    $current_activity = $stat['autochecks']['count'] + $stat['checks']['count'] + $stat['recasts']['count'];
    $total_activity += $current_activity;
    $diff_activity = $current_activity - $activity;

    if ($current_activity > $activity) {
      $tendency = 'green';
    }
    elseif ($current_activity == $activity) {
      $tendency = 'yellow';
    }
    else {
      $tendency = 'red';
    }
    $activity            = $current_activity;
    $rows['key-' . $key] = array(
      'Date'       => $stat['date'],
      'Autochecks' => $stat['autochecks']['count'],
      'Checks'     => $stat['checks']['count'],
      'Recasts'    => $stat['recasts']['count'],
      'Tendency'   => '<span class="mgc-tendency tendency-' . $tendency . '">' . $diff_activity . '</span>',
    );

  }
  $form['activity'] = array(
    '#markup' => '<h5 class="total-activity">Общая активность: ' . $total_activity . '</h5>',
  );

  $duration_start                   = strtotime('first day of ' . date('F Y'));
  $duration_end                     = strtotime('last day of ' . date('F Y'));
  $stop                             = 'Stop';
  $form['duration']['start_date']   = [

    '#date_format'         => 'Y-m-d',
    '#type'                => 'date_popup',
    '#date_label_position' => 'within',
    '#date_timezone'       => date_default_timezone(),
    '#title'               => t('Дата начала выборки'),
    '#default_value'       => date('Y-m-d', $duration_start),

  ];
  $form['duration']['end_date']     = array(

    '#type'                => 'date_popup',
    '#date_format'         => 'Y-m-d',
    '#default_value'       => date('Y-m-d', $duration_end),
    '#date_label_position' => 'within',
    '#title'               => t('Дата конца выборки'),

  );
  $category['selected']             = 0;
  $options                          = array(
    0 => t('No'),
    1 => t('Yes'),
  );
  $form['selected']                 = array(
    '#type'          => 'select',
    '#title'         => t('Selected'),
    '#options'       => array(
      0 => t('No'),
      1 => t('Yes'),
    ),
    '#default_value' => $category['selected'],
    '#description'   => t('Set this to <em>Yes</em> if you would like this category to be selected by default.'),
  );
  $form['manual']['course']         = array(
    '#type'       => 'select',
    '#options'    => $options,
    '#attributes' => array(
      'class' => array(
        'ks-selecter-attach',
      ),
    ),
  );
  $form['actions']['find_activity'] = array(
    '#type'       => 'submit',
    '#value'      => t('Найти'),
    '#attributes' => array(
      'class' => array('table-find-button'),
      // 'id'    => array('sortTable'),
    ),
  );

  $form['table'] = array(
    '#type'       => 'tableselect',
    '#header'     => $header,
    '#options'    => $rows,
    '#empty'      => t('No content available.'),
    '#js_select'  => FALSE,
    //'#multiple'   => TRUE,
    '#attributes' => array(
      'class' => array('tablesorter responsive table-tendency'),
      'id'    => array('sortTable'),
    ),
  );

  $form['table']['#attached']['js'] = array(

    drupal_get_path('theme', 'ditoolsi') . '/js/tablesort/jquery.tablesorter.min.js',
    drupal_get_path('theme', 'ditoolsi') . '/js/common/table-common.js',
  );

  $form['table']['#attached']['css'] = array(
    drupal_get_path('theme', 'ditoolsi') . '/css/table-style.css',
  );


  return $form;
}

function mgc_print_pupil_stats_table_form_submit($form, &$form_state) {
  $form_state['rebuild']                 = TRUE;
  $start_date                            = $form_state['input']['start_date']['date'];
  $end_date                              = $form_state['input']['end_date']['date'];
  $course                                = $form_state['storage']['course'];
  $form_state['storage']['course_stats'] = mgc_get_course_range_stats($course, $start_date, $end_date);
}

function mgc_get_course_range_stats($course, $start_date, $end_date) {
  $course_stats = array();
  $start_date   = strtotime($start_date);
  $end_date     = strtotime($end_date);
  /*Количество дней между датами*/
  $days      = mgc_days_diff($start_date, $end_date);
  $day_start = $start_date;
  $day_end   = $day_start + 24 * 60 * 60 - 1;
  $stop      = 'Stop';
  for ($day = 1; $day <= $days; $day++) {
    $course_stats[$day]['date']       = date('d . m', $day_start);
    $course_stats[$day]['autochecks'] = mgc_get_autochecks_for_day($course, $day, $day_start, $day_end);
    $course_stats[$day]['recasts']    = mgc_get_recasts_for_day($course, $day, $day_start, $day_end);
    /*Начало дня в timestamp*/
    $day_start = $start_date + 24 * 60 * 60 * $day;
    $day_end   = $day_start + 24 * 60 * 60 - 1;
  }

  return $course_stats;
}

function mgc_get_course_stats($course) {
  /*Получаем текущий порядковый день*/
  $today        = idate("d");
  $course_stats = array();
  /*Минус неделя от сегодня*/
  $minus_week = $today - 7;
  /*От первого дня месяца до текущего - собираем статистику*/
  $month_start = strtotime('first day of ' . date('F Y'));
  $day_start   = $month_start + 24 * 60 * 60 * $minus_week;
  $day_end     = $day_start + 24 * 60 * 60 - 1;
  $stop        = 'Stop';
  for ($day = 1; $day <= 7; $day++) {
    $course_stats[$day]['date']       = date('d . m', $day_start);
    $course_stats[$day]['autochecks'] = mgc_get_autochecks_for_day($course, $day, $day_start, $day_end);
    $course_stats[$day]['checks']     = mgc_get_checks_for_day($course, $day, $day_start, $day_end);
    $course_stats[$day]['recasts']    = mgc_get_recasts_for_day($course, $day, $day_start, $day_end);
    /*Начало дня в timestamp*/
    $day_start = $month_start + 24 * 60 * 60 * $minus_week + 24 * 60 * 60 * $day;
    $day_end   = $day_start + 24 * 60 * 60 - 1;
  }

  return $course_stats;
}

function mgc_get_autochecks_for_day($course, $day, $day_start, $day_end) {
  $stop  = 'Stop';
  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'lesson_progress')
    ->propertyCondition('changed', array($day_start, $day_end), 'BETWEEN')
    ->fieldCondition('field_progress_status', 'value', 2);

  $results     = $query->execute();
  $checks_info = array();
  $count       = 0;

  if (isset($results['node'])) {
    foreach ($results['node'] as $result) {
      $progress_nid  = $result->nid;
      $progress_node = node_load($progress_nid);
      if (isset($progress_node->field_progress_lesson_id['und'][0]['target_id'])) {
        $lesson_nid  = $progress_node->field_progress_lesson_id['und'][0]['target_id'];
        $lesson_node = node_load($lesson_nid);
        if (isset($lesson_node->field_lesson_course['und'][0]['target_id']) && $course->nid == $lesson_node->field_lesson_course['und'][0]['target_id']) {
          $stop = 'Stop';
          if (isset($lesson_node->field_lesson_check_type['und'][0]['value']) && $lesson_node->field_lesson_check_type['und'][0]['value'] == '1') {
            $count++;
          }
          /*Если стоит автор проверки, то добавить uid*/
          if (isset($progress_node->field_progress_author_uid['und'][0]['value'])) {
            $stop                                                                                   = 'Stop';
            $checks_info['authors'][$progress_node->field_progress_author_uid['und'][0]['value']][] = '1';
          }
        }
      }


      $stop = 'Stop';
    }
  }
  $checks_info['count'] = $count;

  return $checks_info;
}

function mgc_get_checks_for_day($course, $day, $day_start, $day_end) {
  $stop  = 'Stop';
  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'lesson_progress')
    ->propertyCondition('changed', array($day_start, $day_end), 'BETWEEN')
    ->fieldCondition('field_progress_status', 'value', 2);

  $results     = $query->execute();
  $checks_info = array();
  $count       = 0;

  if (isset($results['node'])) {
    foreach ($results['node'] as $result) {
      $progress_nid  = $result->nid;
      $progress_node = node_load($progress_nid);
      if (isset($progress_node->field_progress_lesson_id['und'][0]['target_id'])) {
        $lesson_nid  = $progress_node->field_progress_lesson_id['und'][0]['target_id'];
        $lesson_node = node_load($lesson_nid);
        if (isset($lesson_node->field_lesson_course['und'][0]['target_id']) && $course->nid == $lesson_node->field_lesson_course['und'][0]['target_id']) {
          $stop = 'Stop';
          if (!isset($lesson_node->field_lesson_check_type['und'][0]['value']) || $lesson_node->field_lesson_check_type['und'][0]['value'] != '1') {
            $count++;
          }
          /*Если стоит автор проверки, то добавить uid*/
          if (isset($progress_node->field_progress_author_uid['und'][0]['value'])) {
            $stop                                                                                   = 'Stop';
            $checks_info['authors'][$progress_node->field_progress_author_uid['und'][0]['value']][] = '1';
          }
        }
      }
      $stop = 'Stop';
    }
  }
  $checks_info['count'] = $count;

  return $checks_info;
}

function mgc_get_recasts_for_day($course, $day, $day_start, $day_end) {
  $stop  = 'Stop';
  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'lesson_progress')
    ->propertyCondition('changed', array($day_start, $day_end), 'BETWEEN')
    ->fieldCondition('field_progress_status', 'value', 3);

  $results     = $query->execute();
  $checks_info = array();
  $count       = 0;

  if (isset($results['node'])) {
    foreach ($results['node'] as $result) {
      $progress_nid  = $result->nid;
      $progress_node = node_load($progress_nid);
      if (isset($progress_node->field_progress_lesson_id['und'][0]['target_id'])) {
        $lesson_nid  = $progress_node->field_progress_lesson_id['und'][0]['target_id'];
        $lesson_node = node_load($lesson_nid);
        if (isset($lesson_node->field_lesson_course['und'][0]['target_id']) && $course->nid == $lesson_node->field_lesson_course['und'][0]['target_id']) {
          $stop = 'Stop';
          $count++;
          /*Если стоит автор проверки, то добавить uid*/
          if (isset($progress_node->field_progress_author_uid['und'][0]['value'])) {
            $stop                                                                                   = 'Stop';
            $checks_info['authors'][$progress_node->field_progress_author_uid['und'][0]['value']][] = '1';
          }
        }
      }
      $stop = 'Stop';
    }
  }
  $checks_info['count'] = $count;

  return $checks_info;
}

function mgc_print_course_stats_by_uid($course, $course_stats) {
  $authors = array();
  foreach ($course_stats as $stats) {
    if (isset($stats['autochecks']['authors'])) {
      foreach ($stats['autochecks']['authors'] as $key => $author) {
        $authors[$key][] = count($author);
      }

    }
    if (isset($stats['checks']['authors'])) {
      foreach ($stats['checks']['authors'] as $key => $author) {
        $authors[$key][] = count($author);
      }

    }
    if (isset($stats['recasts']['authors'])) {
      foreach ($stats['recasts']['authors'] as $key => $author) {
        $authors[$key][] = count($author);
      }

    }
  }
  $html = '';
  if (isset($authors[$course->uid])) {
    $html .= '<div class="teacher-stats">Учителем проверено уроков: ' . array_sum($authors[$course->uid]) . '</div>';
  }
  $html .= '<table class="course-teachers-stats">';
  $html .= '	<thead>
		<tr>
			<th>Куратор</th>
			<th> проверил уроков, шт.</th>
			
		</tr>
	</thead>
	<tbody>';
  foreach ($authors as $key => $author) {
    $checks_sum = array_sum($author);
    $checks_uid = $key;
    $user_node  = user_load($checks_uid);
    $user_name  = $user_node->name;
    /*Если не автор курса, то выводит в таблицу*/

    if ($checks_uid != $course->uid) {
      $html .= ' 
      <tr>
        <td>
   <a href="/id' . $checks_uid . '" > ' . $user_name . '</a>
        </td>
        <td>' . $checks_sum . ' </td>
      </tr>';
    }

  }

  $html .= '</tbody></table>';
  if (isset($authors[$course->uid]) && count($authors) < 2) {
    $html = '<div class="teacher-stats">Учителем проверено уроков: ' . array_sum($authors[$course->uid]) . '</div>';
  }
  else {
    $html = '';
  }

  return $html;


}

function mgc_days_diff($startTimeStamp, $endTimeStamp) {

  $timeDiff   = abs($endTimeStamp - $startTimeStamp);
  $numberDays = $timeDiff / 86400;  // 86400 seconds in one day

// and you might want to convert to integer
  $numberDays = intval($numberDays);

  return $numberDays;
}

function mgc_get_requests_for_review_count() {
  global $user;
  $query = ditoolsi_training_get_cources_query($user->uid, TRUE);
  $nids  = $query->execute()->fetchCol();
  $query = ditoolsi_training_requests_review_query($nids);
  $count = $query->countQuery()->execute()->fetchField();

  return $count;
}

function mgc_get_requests_for_review_w_empty_statuses_count() {
  global $user;
  $query                 = ditoolsi_training_get_cources_query($user->uid, TRUE);
  $nids                  = $query->execute()->fetchCol();
  $query                 = mgc_requests_review_w_empty_statuses_query($nids);
  $results               = $query->execute()->fetchCol();
  $requests              = node_load_multiple($results);
  $request_status_counts = 0;
  foreach ($requests as $request_status) {
    if (isset($request_status->field_progress_custom_status[LANGUAGE_NONE][0]['value']) && $request_status->field_progress_custom_status[LANGUAGE_NONE][0]['value'] != '0') {
      $request_status_counts++;
    }
  }

  return $request_status_counts;
}

function mgc_requests_review_w_empty_statuses_query($course_id = NULL) {
  $query = db_select('node', 'n');
  $query->join('field_data_field_progress_lesson_id', 'lesson_id', 'lesson_id.entity_id = n.nid');
  $query->join('users', 'u', 'u.uid = n.uid');
  $query->join('field_data_field_lesson_number', 'lesson_number', 'lesson_number.entity_id = lesson_id.field_progress_lesson_id_target_id');
  $query->join('field_data_field_lesson_course', 'course', 'course.entity_id = lesson_id.field_progress_lesson_id_target_id');
  $query->join('node', 'node_course', 'node_course.nid = course.field_lesson_course_target_id');
  $query->join('field_data_field_progress_status', 'progress_status', 'progress_status.entity_id = n.nid and progress_status.bundle = :bundle', array(':bundle' => 'lesson_progress'));
  $query
    ->condition('progress_status.field_progress_status_value', DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW)
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

function mgc_start_learn2($course) {
  global $user;

  $query = db_select('ditoolsi_training_pupils_list', 'pl')
    ->fields('pl', array('current_lesson', 'status', 'started'))
    ->fields('lc', array('entity_id'));

  $query->join('node', 'n', 'n.nid = pl.course_id');
  $query->leftJoin('field_data_field_lesson_course', 'lc', 'lc.field_lesson_course_target_id = n.nid');
  $query->leftJoin('field_data_field_lesson_number', 'ln', 'ln.entity_id = lc.entity_id AND ln.field_lesson_number_value = pl.current_lesson');

  $pupil = $query
    ->condition('pl.user_id', $user->uid)
    ->condition('pl.course_id', $course->nid)
    ->range(0, 1)
    ->execute()
    ->fetchObject();

  if (!$pupil || $pupil->started) {
    return MENU_ACCESS_DENIED;
  }

  if (!$pupil->current_lesson) {
    db_update('ditoolsi_training_pupils_list')
      ->fields(array(
        'current_lesson' => 1,
      ))
      ->condition('user_id', $user->uid)
      ->condition('course_id', $course->nid)
      ->execute();

    $pupil->current_lesson = 1;
  }

  if (!$pupil->entity_id) {
    $lesson = dt_training_load_lesson_by_number($course->nid, $pupil->current_lesson);
  }
  else {
    $lesson = node_load($pupil->entity_id);
  }

  if ($lesson) {
    dt_training_update_start_time_progress($lesson, $user);
    db_update('ditoolsi_training_pupils_list')
      ->fields(array('started' => REQUEST_TIME))
      ->condition('user_id', $user->uid)
      ->condition('course_id', $course->nid)
      ->execute();
    //drupal_set_message('Вы можете приступить к обучению');
  }
}

function mgc_start_learn2_from_teacher($course, $user) {

  $query = db_select('ditoolsi_training_pupils_list', 'pl')
    ->fields('pl', array('current_lesson', 'status', 'started'))
    ->fields('lc', array('entity_id'));

  $query->join('node', 'n', 'n.nid = pl.course_id');
  $query->leftJoin('field_data_field_lesson_course', 'lc', 'lc.field_lesson_course_target_id = n.nid');
  $query->leftJoin('field_data_field_lesson_number', 'ln', 'ln.entity_id = lc.entity_id AND ln.field_lesson_number_value = pl.current_lesson');

  $pupil = $query
    ->condition('pl.user_id', $user->uid)
    ->condition('pl.course_id', $course->nid)
    ->range(0, 1)
    ->execute()
    ->fetchObject();

  if (!$pupil || $pupil->started) {
    return MENU_ACCESS_DENIED;
  }

  if (!$pupil->current_lesson) {
    db_update('ditoolsi_training_pupils_list')
      ->fields(array(
        'current_lesson' => 1,
      ))
      ->condition('user_id', $user->uid)
      ->condition('course_id', $course->nid)
      ->execute();

    $pupil->current_lesson = 1;
  }

  if (!$pupil->entity_id) {
    $lesson = dt_training_load_lesson_by_number($course->nid, $pupil->current_lesson);
  }
  else {
    $lesson = node_load($pupil->entity_id);
  }

  if ($lesson) {
    dt_training_update_start_time_progress($lesson, $user);
    db_update('ditoolsi_training_pupils_list')
      ->fields(array('started' => REQUEST_TIME))
      ->condition('user_id', $user->uid)
      ->condition('course_id', $course->nid)
      ->execute();
    //drupal_set_message('Вы можете приступить к обучению');
  }
}

function mgc_user_has_role($rid, $account = NULL) {
  if (!$account) {
    $account = $GLOBALS['user'];
  }

  return isset($account->roles[$rid]);
}

function mgc_user_has_curator_role($account = NULL) {
  if (!$account) {
    $account = $GLOBALS['user'];
  }
  $rid = 5;

  return isset($account->roles[$rid]);
}

function mgc_user_has_teacher_role($account = NULL) {
  if (!$account) {
    $account = $GLOBALS['user'];
  }
  $rid = 6;

  return isset($account->roles[$rid]);
}

function mgc_get_curator_course($user) {
  $is_curator = db_select('ditoolsi_training_curators_list', 'cl')
    ->fields('cl', array('course_id'))
    ->condition('cl.user_id', $user->uid)
    ->execute()
    ->fetchField();

  return $is_curator;
}
function mgc_update_question_field(){
  mgc_change_text_field_max_length('field_autotest_questions', 20000);

}
function mgc_change_text_field_max_length($field_name, $new_length) {
  $field_table = 'field_data_' . $field_name;
  $field_revision_table = 'field_revision_' . $field_name;
  $field_column = $field_name . '_value';

  // Alter value field length in fields table
  db_query("UPDATE `{$field_table}` SET `{$field_column}`=SUBSTR(`{$field_column}`, 0, {$new_length})");
  db_query("ALTER TABLE `{$field_table}` CHANGE `{$field_column}` `{$field_column}` VARCHAR( {$new_length} )");
  // Alter value field length in fields revision table
  db_query("UPDATE `{$field_revision_table}` SET `{$field_column}`=SUBSTR(`{$field_column}`, 0, {$new_length})");
  db_query("ALTER TABLE `{$field_revision_table}` CHANGE `{$field_column}` `{$field_column}` VARCHAR( {$new_length} )");

  // Update field config with new max length
  $result = db_query("SELECT CAST(`data` AS CHAR(10000) CHARACTER SET utf8) FROM `field_config` WHERE field_name = '{$field_name}'");
  $config = $result->fetchField();
  $config_array = unserialize($config);
  $config_array['settings']['max_length'] = $new_length;
  $config = serialize($config_array);
  db_update('field_config')
    ->fields(array('data' => $config))
    ->condition('field_name', $field_name)
    ->execute();
}
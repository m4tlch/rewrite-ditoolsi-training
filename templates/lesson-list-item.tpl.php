<?php
$wrapper = entity_metadata_wrapper('node', $lesson);
$course_id = $wrapper->field_lesson_course->raw();
$course = node_load($course_id);
$open_lesson = $course->field_if_lesson_open[LANGUAGE_NONE][0]['value'];
$lesson_number = $wrapper->field_lesson_number->value();
$account_obj = (array) $account;
$visible = DT_TRAINING_LESSON_VISIBLE_VISIBLE_ALL_ACCESS_ONE;

if (isset($course->field_course_lessons_visible[LANGUAGE_NONE][0]['value'])) {
  $visible = $course->field_course_lessons_visible[LANGUAGE_NONE][0]['value'];
}

if (empty($account_obj)) {
  $account = $user;
}

switch ($visible) {
  case DT_TRAINING_LESSON_VISIBLE_VISIBLE_ONE_ACCESS_ONE:
    $allow_view = ($current_lesson == $lesson_number || $open_lesson) && $current_lesson != -2;
    break;

  case DT_TRAINING_LESSON_VISIBLE_VISIBLE_ALL_ACCESS_ALL:
    $allow_view = TRUE;
    break;

  case DT_TRAINING_LESSON_VISIBLE_VISIBLE_MANUAL_ACCESS_MANUAL:
    $c_wrapper = entity_metadata_wrapper('node', $course);
    $ids = $c_wrapper->field_course_lessons_list->raw();

    $allow_view = in_array($lesson->nid, $ids) || ($pupil && $pupil->current_lesson >= $lesson_number);
    break;

  default:
  case DT_TRAINING_LESSON_VISIBLE_VISIBLE_ALL_ACCESS_ONE:
    $allow_view = (($current_lesson == $lesson_number) || ($open_lesson && !($current_lesson < $lesson_number))) && $current_lesson != -2;
    break;
}

$allow_view = $allow_view && ditoolsi_training_access('pass lesson', $course) && ditoolsi_training_get_cource_access($course, FALSE);
$allow_view = $allow_view && !empty($pupil->started);
$started = !empty($pupil->started) ? $pupil->started : -1;

$timer_allow = FALSE;

if (empty($lesson->progress) || in_array($lesson->progress->field_progress_status->value(), array(
  DITOOLSI_TRAINING_PROGRESS_STATUS_NEW,
  DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST,
  DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW,
  ))) {

  $timer_allow = TRUE;
}
?>
<div class="lesson-list-item clearfix">
  <div class="lesson-number">
    <span>
      <?php
        print $lesson_number;
      ?>
    </span>
  </div>
  <div class="lesson-info">
    <div class="lesson-title">
      <?php
        if ($history) {
          print l($lesson->title, 'course/' . $course_id . '/' . $lesson->nid . '/' . $account->uid . '/view-history');
        }
        elseif ($allow_view) {
          print l($lesson->title, 'course/' . $course_id . '/' . $lesson->nid);
        }
        else {
          print '<span data-pass="' . ((int) ditoolsi_training_access('pass lesson', $course)) . '" data-access="' . $access_request . '" data-started="' . $started . '" data-course-id="' . $course_id . '">' . $lesson->title . '</span>';
        }

        if (empty($hide_status)) {
          $lesson_status = ditoolsi_training_lesson_status_view($lesson, $allow_view);
          print $lesson_status;

          if ($allow_view && !empty($lesson->field_lesson_pass_time[LANGUAGE_NONE][0]['value']) && $lesson->field_lesson_number[LANGUAGE_NONE][0]['value'] == $pupil->current_lesson && $timer_allow) {
            $time_pass = $lesson->field_lesson_pass_time[LANGUAGE_NONE][0]['value'];
            $time_start = !empty($lesson->progress) ? $lesson->progress->field_progress_time_start->value() : 0;
            print ($time_pass && $time_start ? '<div class="timer">Осталось времени: <span>' . (($time_pass*24*3600 + $time_start) - REQUEST_TIME) . '</span></div>' : '');
          }
        }
      ?>
      <span class="edit-link">
        <?php
        if(array_key_exists(DITOOLSI_CURATOR_RID, $account->roles)
          || array_key_exists(DITOOLSI_ADMIN_RID, $account->roles)
          || array_key_exists(DITOOLSI_TEACHER_RID, $account->roles)) {
          print l('&nbsp;', 'course/' . $course_id . '/' . $lesson->nid . '/edit', array(
            'html'  => TRUE,
          ));
        }
        ?>
      </span>
    </div>
    <div class="lesson-stat">
      <?php
        $info = ditoolsi_training_lesson_attach_info($lesson);
        print $info;
      ?>
    </div>
  </div>
</div>

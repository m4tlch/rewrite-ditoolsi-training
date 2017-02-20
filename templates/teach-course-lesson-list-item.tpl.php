<?php
$wrapper       = entity_metadata_wrapper('node', $lesson);
$course_id     = $wrapper->field_lesson_course->raw();
$course        = node_load($course_id);
$open_lesson   = $course->field_if_lesson_open[LANGUAGE_NONE][0]['value'];
$lesson_number = $wrapper->field_lesson_number->value();
$account_obj   = (array) $account;
$visible       = DT_TRAINING_LESSON_VISIBLE_VISIBLE_ALL_ACCESS_ONE;

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
    $ids       = $c_wrapper->field_course_lessons_list->raw();

    $allow_view = in_array($lesson->nid,
        $ids) || ($pupil && $pupil->current_lesson >= $lesson_number);
    break;

  default:
  case DT_TRAINING_LESSON_VISIBLE_VISIBLE_ALL_ACCESS_ONE:
    $allow_view = (($current_lesson == $lesson_number) || ($open_lesson && !($current_lesson < $lesson_number))) && $current_lesson != -2;
    break;
}

$allow_view = $allow_view && mgc_user_has_teacher_role();
$started    = !empty($pupil->started) ? $pupil->started : -1;

$timer_allow = FALSE;

if (empty($lesson->progress) || in_array($lesson->progress->field_progress_status->value(),
    array(
      DITOOLSI_TRAINING_PROGRESS_STATUS_NEW,
      DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST,
      DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW,
    ))
) {

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
            print l($lesson->title,
              'teach-course/' . $course_id . '/' . $lesson->nid . '/' .
              $account->uid . '/view-history');
          }
          elseif ($allow_view) {
            print l($lesson->title,
              'teach-course/' . $course_id . '/' . $lesson->nid);
          }
          else {
            print '<span data-pass="' . ((int) ditoolsi_training_access('pass lesson',
                $course)) . '" data-access="' . $access_request . '" data-started="' . $started . '" data-course-id="' . $course_id . '">' . $lesson->title . '</span>';
          }

          if (empty($hide_status)) {
            $lesson_status = mgc_teach_course_lesson_status_view($lesson,
              $allow_view);
            print $lesson_status;
          }
          ?>
        </div>
        <div class="lesson-stat">
          <?php
          $info = ditoolsi_training_lesson_attach_info($lesson);
          print $info;
          ?>
        </div>
    </div>
</div>

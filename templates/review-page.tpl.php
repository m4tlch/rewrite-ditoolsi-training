<?php
$courses = ditoolsi_training_get_courses($node->uid);
$info    = ditoolsi_training_lesson_attach_info($node);
?>

<div class="all-courses-dropdown dropdown">
  <div class="all-courses-btn dropdown-toggle" data-toggle="dropdown"
       id="list-courses-dropdown">
    <span>
      <i class="fa fa-bars"></i>
      <?php print t('All courses'); ?>
    </span>
  </div>


  <ul class="dropdown-menu" role="menu" aria-labelledby="list-courses-dropdown">
    <li role="presentation">
      <?php
      print l(t('All courses'), 'courses', array(
        'attributes' => array(
          'class'    => array('bold'),
          'role'     => 'menuitem',
          'tabindex' => -1,
        ),
      ));
      ?>
    </li>
    <?php
    foreach ($courses as $course_item) {
      print '
          <li role="presentation">
            ' . l($course_item->title, 'course/' . $course_item->nid, array(
          'attributes' => array(
            'role'     => 'menuitem',
            'tabindex' => -1,
          ),
        )) . '
          </li>';
    }
    ?>
  </ul>
</div>

<div class="info-label">
  <?php
  print t('Information about lesson');
  ?>
</div>
<div class="course-title">
  <?php
  print t('Course') . ': ' . l($course->title, "node/{$course->nid}");
  ?>
</div>
<div class="lesson-header">
  <div class="lesson-number">
    <?php
    print $wrapper->field_lesson_number->value();
    ?>
  </div>
  <div class="lesson-info">
    <div class="lesson-title">
      <?php
      print l($node->title, 'course/' . $course->nid . '/' . $node->nid);
      ?>
    </div>
    <div class="lesson-stat">
      <?php
      print $info;
      ?>
    </div>
  </div>
</div>
<div class="lesson-body">
  <?php
  $body = $wrapper->body->value();
  print check_markup($body['value'], $body['format']);
  ?>
</div>

<div id="lesson-tabs">

  <ul>
    <li>
      <a href="#to-task">
        <?php
        print t('Task');
        ?>
      </a>
    </li>
  </ul>

  <div id="to-task">
    <?php
    $transfer_form = ditoolsi_training_get_transfer_lesson_form($node,
      $progress);
    print render($transfer_form);
    $task_statuses = ditoolsi_training_get_task_status_for_review($node,
      $progress);
    ?>
    <ul id="task-switcher-tabs">
      <?php
      $tasks = array_values($node->tasks);
      $i     = 0;
      foreach ($tasks as $key => $task) {
        if ($task->type != 'lesson_text_task') {
          $stop = 'Stop';
          if (count($tasks) > 1) {
            continue;
          }

        }
        $i++;
        $status_class = 'status-new';
        if (!empty($task_statuses[$task->nid])) {
          $status_class = ditoolsi_training_get_task_status_class($task_statuses[$task->nid]);
        }

        print '
            <li>
              <a href="#to-task-' . $key . '"  class="' . $status_class . '">' . $i . '</a>
            </li>';
      }
      ?>
    </ul>

    <?php

    foreach ($tasks as $key => $task) {
      if ($task->type != 'lesson_text_task') {
        continue;
      }

      $t_wrapper = entity_metadata_wrapper('node', $task);
      $form      = drupal_get_form('ditoolsi_training_save_task_form', $node,
        $task, $progress);

      $body = $t_wrapper->body->value();
      print '<div id="to-task-' . $key . '" class="task-tab-content">';

      if ($t_wrapper->field_text_task_redaction->value()) {
        $link = l(t('Go to task'),
          'course/' . $course->nid . '/' . $node->nid . '/' . $progress->nid .
          '/review-advanced/' . $task->nid);
        print '<div class="task-body">' . $link . '</div>';
        print '</div>';
      }
      else {
        print '<div class="task-body">' . check_markup($body['value'],
            $body['format']) . '</div>';
        print '<div class="accept-task-form">';
        print render($form);
        print '</div>';
        $form = drupal_get_form('ditoolsi_training_to_fix_task_form', $node,
          $task, $progress);
        print '<div class="to-fix-task-form element-hidden">';
        print render($form);
        print '</div>';
        print '</div>';
      }
    }
    ?>
  </div>

</div>
<?php
  $info =ditoolsi_training_lesson_attach_info($node);
  $courses = ditoolsi_training_get_courses($node->uid);
?>

<div class="all-courses-dropdown dropdown">
  <div class="all-courses-btn dropdown-toggle" data-toggle="dropdown" id="list-courses-dropdown">
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
<div class="course-id">
  id<?php print $node->nid; ?>
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

<div class="task-header">
  <?php if (!empty($pupil)): ?>
    <div class="task-pupil">
      <span><?php print t('Pupil'); ?>:</span>
      <?php print $pupil; ?>
    </div>
  <?php endif ?>
  <div class="task-body-title">
    <?php
      print t('Task');
    ?>:
  </div>
  <div class="task-body">
    <?php
      print $body;
    ?>
  </div>
  <div class="task-file">
    <span><?php print t('File to task'); ?>:</span>
    <?php
      print $file_url;
    ?>
  </div>
</div>
<div class="task-content clearfix">
  <div class="task-answers">
    <div id="task-redaction">

    </div>
    <div id="task-answer">
      <?php
        print render($form);
      ?>  
    </div>
  </div>
  <ul class="redaction-list">
    
    <?php
      $active_class = '';
      if ($history) {
        $active_class = 'active';
      }
      foreach ($redactions as $key => $value) {
        print '<li class="redaction-item">';
        print l(t('Redaction @redaction', array(
          '@redaction' => $key)
          ), 'task-redaction/' . $value, array(
            'attributes' => array(
              'class' => array(
                'redaction-task',
                'use-ajax',
                $active_class,
              ),
            ),
          )
        );
        print '</li>';
        $active_class = '';
      }
      if (!$history) {
        print '<li class="redaction-item">';
        print l(t('Current redaction'), current_path(), array(
            'fragment' => 'task-answer',
            'attributes' => array(
              'class' => array(
                'current-task',
              ),
            ),
          )
        );
        print '</li>';
      }
    ?> 
    </li>  
  </ul>  
</div>

<?php

	if (!empty($count_time) && !empty($course->lessons)) {
		$counts = count($course->lessons);
		$procent_finished = ($count_time / $counts)*100;
	  $procent_finished = round($procent_finished);
	  $forecast_time = $counts * $middle_time;
	}
	else {
		$counts = NULL;
		$procent_finished = NULL;
	  $procent_finished = NULL;
	  $forecast_time = NULL;
	}
?>
<div class="course achievements-pupil">
	<div class="course-block">
	  <?php
	    print theme('ditoolsi_training_course_block', array(
	      'node'            => $course,
	      'picture'         => $wrapper->field_course_image->value(),
	      'type'            => $wrapper->field_course_type->value(),
	      'specialization'  => $wrapper->field_course_specialization->value(),
	      'time_completion' => $wrapper->field_course_time_completion->value(),
	      'sn_group'        => $wrapper->field_course_sn_group->value(),
	      'wrapper'         => $wrapper,
	      'pupil'           => $pupil,
	    ));
	  ?>
	</div>
	<div class="course-content">
	  <div class="course-title">
	    <?php
	      print $course->title;
	    ?>
	  </div>
	<div class="course-id">
	  id<?php print $course->nid; ?>
	</div>

	  <div class="course-body">
	    <?php
	      print $description;
	    ?>
	  </div>
	</div>
</div>
<div class="c_statist">
  <?php
  if (ditoolsi_training_access('toggle statistic block', $course)) {
    if (!empty($course->field_course_visible_sb[ LANGUAGE_NONE ][ 0 ][ 'value' ])) {
      $link_title = 'Hide block with statistics';
    }
    else {
      $link_title = 'Show block with statistics';
    }

    print l(t($link_title), 'toggle-statistic-block/' . $course->nid, array(
      'attributes' => array(
        'class' => array('use-ajax'),
        'id' => 'toggle-statistic-block'
      ),
    ));
  }

    if (!empty($course->field_course_visible_sb[LANGUAGE_NONE][0]['value'])):
  ?>

  <div id="statistic-block">
    <?php
    print theme('ditoolsi_training_statistic_block', array(
      'middle_time' => $middle_time,
      'optimal_temp' => $optimal_temp,
      'forecast_time' => $forecast_time,
      'count_time' => $count_time,
      'procent_finished' => $procent_finished,
    ));
    ?>
  </div>


  <?php endif; ?>
</div>



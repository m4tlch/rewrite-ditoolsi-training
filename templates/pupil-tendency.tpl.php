<?php
$i = 0;
?>
<div class="pupil-tendency-page">
  <div class="course-inform">
    <div class="course-title"><?php print $course->title; ?></div>
    <div class="course-nid">id<?php print $course->nid; ?></div>
  </div>
  <div class="course-teachers">
    <div class="course-teachers-title">Статистика активности по учителю и кураторам за период времени
    </div>
  </div>
  <div class="tendency">

    <?php
    //mgc_print_pupil_stats_table($course);
    $form = drupal_get_form('mgc_print_pupil_stats_table_form', $course);
    print render($form);
    ?>

  </div>

</div>

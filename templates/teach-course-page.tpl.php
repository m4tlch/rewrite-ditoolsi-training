<div class="course-block">
  <?php
  print theme('ditoolsi_training_course_block', array(
    'node'            => $node,
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
      print $node->title;
      ?>
    </div>

    <div class="course-body">
      <?php
      print $description;
      ?>
    </div>
</div>

<div class="modal fade" id="myModal" tabindex="-1" role="dialog"
     aria-labelledby="exampleModalLabel">
    <div class="modal-dialog" role="document">
        <div class="modal-content">
            <a class="close-video" data-dismiss="modal" aria-hidden="true">

            </a>
            <div class="video">
              <?php
              $field = field_get_items('node', $node, 'field_course_video');

              if ($field && isset($user->roles[DITOOLSI_PUPIL_RID])) {
                $display             = array(
                  'type'     => 'youtube_video',
                  'settings' => array(
                    'size'     => '960x720',
                    'autoplay' => TRUE,
                  ),
                );
                $output              = field_view_value('node', $node,
                  'field_course_video', $field[0], $display);
                $output['#size']     = '640x480';
                $output['#autoplay'] = TRUE;
                print "<div id='video'></div>";
                print "<div id='vid_id'>" . $output['#video_id'] . "</div>";
              }
              ?>
            </div>
        </div>
    </div>
</div>
<div class="course-lessons">
    <div class="lessons-quantity">
      <?php
      print t('Chain lessons: !count', array(
        '!count' => '<span>' . count($lessons) . '</span>',
      ));
      ?>
    </div>
    <div class="lessons-list">
      <?php
      $access_request = ditoolsi_training_access('request learning', $node);

      if ($pupil && !$pupil->started) {
        print l('Начать обучение', "course/{$node->nid}/start-learn2", array(
          'attributes' => array(
            'class' => array(
              'dit-submit-blue btn start-learn',
            ),
          ),
        ));
      }

      foreach ($lessons as $lesson) {
        print theme('mgc_teach_course_lesson_list_item', array(
          'lesson'         => $lesson,
          'title'          => $lesson->title,
          'nid'            => $lesson->nid,
          'current_lesson' => $current_lesson,
          'access_request' => $access_request,
          'pupil'          => $pupil,
        ));
      } ?>
    </div>
</div>
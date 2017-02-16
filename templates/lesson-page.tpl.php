<?php
drupal_add_library('system', 'drupal.ajax');
$courses        = ditoolsi_training_get_courses($node->uid);
$videos         = $wrapper->field_lesson_video->value();
$i              = 0;
$videos_tabs    = array();
$videos_content = array();
$warning        = '
    <div class="notification">
      ' . t('Warning: Video may be loaded for a long time, it depends on the speed <br>
Your Internet. If the download is a long time, just wait!') . '
    </div>
  ';

foreach ($videos as $key => $video) {
  $i++;

  $v_wrapper  = entity_metadata_wrapper('field_collection_item', $video);
  $video_type = $v_wrapper->field_lesson_video_type->value();
  $stop       = 'Stop';
  switch ($video_type) {
    case DITOOLSI_TRAINING_VIDEO_TYPE_YOUTUBE:
      $video = $v_wrapper->field_lesson_video_url->value();

      if ($video && preg_match('/[\\?\\&]v=([^\\?\\&]+)/', $video, $matches)) {
        $title = $v_wrapper->field_lesson_video_title->value();

        if ($title) {
          $title = '<div class="title">' . $title . '</div>';
        }

        $videos_tabs[]    = '<a href="#to-video-' . $key . '">' . $i . '</a>';
        $videos_content[] = array(
          '#markup' => '
              <div id="to-video-' . $key . '" class="video-tab-content">
                ' . $warning . '<br>
                ' . $title . '
                <iframe src="http://www.youtube.com/embed/' . $matches[1] . '" frameborder="0" allowfullscreen></iframe>
              </div>
            ',
        );
      }
      break;
    case DITOOLSI_TRAINING_VIDEO_TYPE_CODE:
      $video = $v_wrapper->field_embedded_code->value();
      $stop  = 'Stop';
      if ($video) {
        $title = $v_wrapper->field_lesson_video_title->value();

        if ($title) {
          $title = '<div class="title">' . $title . '</div>';
        }

        $videos_tabs[]    = '<a href="#to-video-' . $key . '">' . $i . '</a>';
        $videos_content[] = array(
          '#markup' => '
              <div id="to-video-' . $key . '" class="video-tab-content">
                ' . $warning . '<br>' . $title . '<div class="lesson-embedded-video">' . $video . '</div></div>',
        );
      }
      break;
    case DITOOLSI_TRAINING_VIDEO_TYPE_LOCAL:
      $video = $v_wrapper->field_lesson_video_file->value();

      if ($video) {
        $title = $v_wrapper->field_lesson_video_title->value();

        if ($title) {
          $title = '<div class="title">' . $title . '</div>';
        }

        $video_url        = file_create_url($video['uri']);
        $videos_tabs[]    = '<a href="#to-video-' . $key . '">' . $i . '</a>';
        $videos_content[] = array(
          '#markup' => '
              <div id="to-video-' . $key . '" class="video-tab-content">
                ' . $warning . '<br>
                ' . $title . '
                <video controls>
                  <source src="' . $video_url . '" type="video/mp4">
                </video>
              </div>
            ',
        );
      }
      break;
  }
}

$videos_quantity = count($videos_content);

$audios        = $wrapper->field_lesson_audio->value();
$i             = 0;
$audio_tabs    = array();
$audio_content = array();

foreach ($audios as $key => $audio) {
  $i++;

  $v_wrapper = entity_metadata_wrapper('field_collection_item', $audio);
  $file      = $v_wrapper->field_lesson_audio_file->value();

  if ($file) {
    $title = $v_wrapper->field_lesson_audio_title->value();

    if ($title) {
      $title = '<div class="title">' . $title . '</div>';
    }

    $audio_url       = file_create_url($file['uri']);
    $audio_tabs[]    = '<a href="#to-audio-' . $key . '">' . $i . '</a>';
    $audio_content[] = array(
      '#markup' => '
          <div id="to-audio-' . $key . '" class="audio-tab-content">
            ' . $title . '
            <audio controls>
              <source src="' . $audio_url . '" type="audio/mpeg">
            </audio>
          </div>
        ',
    );
  }
}

$audios_quantity = count($audio_content);

$presentations         = $wrapper->field_lesson_presentation->value();
$i                     = 0;
$presentations_tabs    = array();
$presentations_content = array();

foreach ($presentations as $key => $presentation) {
  $i++;

  $p_wrapper    = entity_metadata_wrapper('field_collection_item',
    $presentation);
  $presentation = $p_wrapper->field_lesson_presentation_file->value();

  if ($presentation) {
    $title = $p_wrapper->field_lesson_presentation_title->value();

    if ($title) {
      $title = '<div class="title">' . $title . '</div>';
    }

    $url                     = file_create_url($presentation['uri']);
    $presentations_tabs[]    = '<a href="#to-presentation-' . $key . '">' . $i . '</a>';
    if($presentation['filemime'] == 'application/pdf'){
      $presentations_content[] = array(
        '#markup' => '
          <div id="to-presentation-' . $key . '" class="presentation-tab-content">
            ' . $title . '
            <iframe src="' . $url. '"></iframe>
          </div>
        ',
      );
    }
    else{
      $presentations_content[] = array(
        '#markup' => '
          <div id="to-presentation-' . $key . '" class="presentation-tab-content">
            ' . $title . '
            <iframe src="http://docs.google.com/gview?url=' . $url . '&embedded=true" frameborder="0"></iframe>
          </div>
        ',
      );
    }

  }
}

$presentations_quantity = count($presentations_content);

$is_lesson_finished = ditoolsi_training_is_lesson_complete($node, $account);
$allowed_for_review = ditoolsi_training_lesson_allowed_for_review($node,
  $account);
$tasks              = array_values($node->tasks);
$task_nids          = array_keys($node->tasks);

if ($task_nids) {
  $query = db_select('node', 'n');
  $query->join('field_data_field_progress_lesson_id', 'lesson_id',
    'lesson_id.entity_id = n.nid');
  $query->join('field_data_field_progress_tasks', 'progress_tasks',
    'progress_tasks.entity_id = n.nid');
  $query->join('field_data_field_progress_task_progress', 'task_progress',
    'task_progress.entity_id = progress_tasks.field_progress_tasks_value');
  $query->join('field_data_field_progress_status', 'progress_status',
    'progress_status.entity_id = progress_tasks.field_progress_tasks_value');
  $query->join('node', 'node_progress_task',
    'node_progress_task.nid = task_progress.field_progress_task_progress_target_id');
  $query->join('field_data_field_progress_task', 'progress_task',
    'progress_task.entity_id = node_progress_task.nid');
  $query->join('node', 'node_task',
    'node_task.nid = progress_task.field_progress_task_target_id');
  $query->leftJoin('field_data_field_progress_blocked', 'blocked',
    'blocked.entity_id = node_progress_task.nid');

  $query
    ->fields('node_task', array('nid'))
    ->fields('blocked', array('field_progress_blocked_value'))
    ->fields('progress_status', array('field_progress_status_value'))
    ->condition('n.type', 'lesson_progress')
    // ->condition('node_progress_task.type', 'auto_test_progress')
    // ->condition('node_task.type', 'lesson_auto_test')
    ->condition('node_task.nid', $task_nids, 'IN')
    ->condition('n.uid', $account->uid)
    ->condition('node_progress_task.uid', $account->uid);

  $items = $query->execute()->fetchAll();
}
else {
  $items = array();
}

$progress = array();

foreach ($items as $item) {
  $progress[$item->nid] = $item;
}
?>
    <div class="all-courses-dropdown dropdown">
        <div class="all-courses-btn dropdown-toggle" data-toggle="dropdown"
             id="list-courses-dropdown">
    <span>
      <i class="fa fa-bars"></i>
      <?php print t('All courses'); ?>
    </span>
        </div>


        <ul class="dropdown-menu" role="menu"
            aria-labelledby="list-courses-dropdown">
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
    <div class="modal fade" id="myModal" tabindex="-1" role="dialog"
         aria-labelledby="exampleModalLabel">
        <div class="modal-dialog" role="document">
            <div class="modal-content">
                <a class="close-video" data-dismiss="modal" aria-hidden="true">

                </a>
                <div class="video">
                  <?php
                  $node_course = node_load(arg(1));
                  $field       = field_get_items('node', $node_course,
                    'field_course_video');

                  if ($field && isset($user->roles[DITOOLSI_PUPIL_RID])) {
                    $output              = field_view_value('node',
                      $node_course,
                      'field_course_video', $field[0]);
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
    <div class="lesson-value">
        <div class="info-title-lesson">
          <?php
          print $node->title;
          ?>
        </div>
        <div class="course-id">
            id <?php
          print $node->nid;
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
                  print l($node->title,
                    'course/' . $course->nid . '/' . $node->nid);
                  ?>
                </div>
                <div class="lesson-stat">
                  <?php
                  $info = ditoolsi_training_lesson_attach_info($node);
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
    </div>
    <div id="lesson-tabs">
      <?php
      $tabs             = array();
      $video_tab        = array();
      $audio_tab        = array();
      $presentation_tab = array();

      if ($videos_quantity) {
        $tabs[]         = '<a href="#to-video">' . t('Video') . '</a>';
        $video_tab_link = theme('item_list', array(
          'items'      => $videos_tabs,
          'attributes' => array(
            'id' => 'video-switcher-tabs',
          ),
        ));
        $video_tab[]    = array(
          '#markup' => $video_tab_link,
        );
        $video_tab[]    = $videos_content;
      }

      if ($audios_quantity) {
        $tabs[]         = '<a href="#to-audio">' . t('Audio') . '</a>';
        $audio_tab_link = theme('item_list', array(
          'items'      => $audio_tabs,
          'attributes' => array(
            'id' => 'audio-switcher-tabs',
          ),
        ));
        $audio_tab[]    = array(
          '#markup' => $audio_tab_link,
        );
        $audio_tab[]    = $audio_content;
      }

      if ($presentations_quantity) {
        $tabs[]                = '<a href="#to-presentation">' . t('Presentation') . '</a>';
        $presentation_tab_link = theme('item_list', array(
          'items'      => $presentations_tabs,
          'attributes' => array(
            'id' => 'presentation-switcher-tabs',
          ),
        ));
        $presentation_tab[]    = array(
          '#markup' => $presentation_tab_link,
        );
        $presentation_tab[]    = $presentations_content;
      }

      $tabs[] = '<a href="#to-task">' . t('Task') . '</a>';

      print theme('item_list', array(
        'items' => $tabs,
      ));

      if ($video_tab) {
        print '<div id="to-video">';
        print render($video_tab);
        print '</div>';
      }

      if ($audio_tab) {
        print '<div id="to-audio">';
        print render($audio_tab);
        print '</div>';
      }

      if ($presentation_tab) {
        print '<div id="to-presentation">';
        print render($presentation_tab);
        print '</div>';
      }
      ?>

        <div id="to-task">
            <ul id="task-switcher-tabs">
              <?php
              $i                   = 0;
              $task_progress_count = 0;
              $total_task_count    = count($tasks);
              foreach ($tasks as $key => $task) {
                $i++;
                $status_class = 'status-new';
                if (isset($progress[$task->nid])) {
                  $task_progress_count++;
                }


                if (!empty($progress[$task->nid]->field_progress_status_value)) {
                  $status_class = ditoolsi_training_get_task_status_class($progress[$task->nid]->field_progress_status_value);
                }

                print '
            <li>
              <a href="#to-task-' . $key . '" class="' . $status_class . '">' . $i . '</a>
            </li>';
              }
              print '</ul>';

              $data_count = $total_task_count - $task_progress_count;
              print '<span class="task-progress-count-invisible" data-count="' . $data_count . '"></span>';
              ?>


              <?php

              $text = ditoolsi_training_locked_autotest();
              $stop = 'Stop';
              if ($history) {
                $i = 0;
                foreach ($tasks as $task) {
                  $t_wrapper    = entity_metadata_wrapper('node', $task);
                  $status_class = '';
                  $status       = FALSE;
                  $form         = '';

                  if (empty($progress[$task->nid])) {
                    continue;
                  }
                  $task_answer = $progress[$task->nid];
                  print '<div id="to-task-' . $i . '" class="task-tab-content">';
                  $i++;

                  switch ($task->type) {
                    case 'lesson_auto_test':
                      print '<div id="autotest-' . $task->nid . '">
                      <div class="status-text green">' . t('The test is complete') . '</div>
                    </div>';
                      break;

                    case 'lesson_text_task':
                      if ($t_wrapper->field_text_task_redaction->value()) {
                        $query = new EntityFieldQuery();
                        $query
                          ->entityCondition('entity_type', 'node')
                          ->entityCondition('bundle', 'text_task_progress')
                          ->propertyCondition('uid', $account->uid)
                          ->fieldCondition('field_progress_task', 'target_id',
                            $task->nid);

                        $result       = $query->execute();
                        $progress_nid = key($result['node']);
                        $link         = l(t('Go to task'),
                          'course/' . $course->nid . '/' .
                          $node->nid . '/' . $progress_nid . '/review-advanced/' . $task_answer->nid);
                        print '<div class="task-body">' . $link . '</div>';
                      }
                      else {
                        $body = $t_wrapper->body->value();
                        print '<div class="task-body">' . check_markup($body['value'],
                            $body['format']) . '</div>';
                        $form = ditoolsi_training_text_task_history($node,
                          $task,
                          $account);
                      }
                      break;
                  }

                  print render($form);
                  print '</div>';
                }
              }
              else {
                foreach ($tasks as $key => $task) {
                  $t_wrapper    = entity_metadata_wrapper('node', $task);
                  $status_class = '';
                  $status       = FALSE;
                  $form         = '';

                  print '<div id="to-task-' . $key . '" class="task-tab-content">';

                  switch ($task->type) {
                    case 'lesson_auto_test':
                      print '<div id="autotest-' . $task->nid . '">';

                      if (!empty($progress[$task->nid]->field_progress_status_value)) {
                        $status = $progress[$task->nid]->field_progress_status_value;
                      }

                      if ($status == DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED) {
                        print '<div class="status-text green">' . t('The test is complete') . '</div>';
                      }
                      elseif ($current_lesson > $wrapper->field_lesson_number->value() && !empty($progress)) {
                        print '<div class="status-text green">' . t('The test is complete') . '</div>';
                      }
                      /*
                      * empty($progress) || empty($progress[$task->nid]->field_progress_blocked_value) || (isset($progress[$task->nid]->field_progress_blocked_value)&&$progress[$task->nid]->field_progress_blocked_value!=="1")
                      * */
                      elseif (empty($progress[$task->nid]->field_progress_blocked_value)) {
                        print l(t('Start testing'),
                          'task/' . $task->nid . '/autotest', array(
                            'attributes' => array(
                              'class' => array(
                                'btn',
                                'btn-default',
                                'dit-submit-blue',
                                'use-ajax',
                              ),
                            ),
                          ));
                      }
                      else {
                        print '<div class="status-text red">' . $text . '</div>';
                      }

                      print '</div>';
                      break;

                    case 'lesson_text_task':
                      if ($t_wrapper->field_text_task_redaction->value()) {
                        $link = l(t('Go to task'),
                          'course/' . $course->nid . '/' . $node->nid . '/task-advanced/' . $task->nid);
                        print '<div class="task-body">' . $link . '</div>';
                      }
                      else {
                        $body = $t_wrapper->body->value();
                        print '<div class="task-body">' . check_markup($body['value'],
                            $body['format']) . '</div>';
                        $task_node_nid = $t_wrapper->getIdentifier();
                        $task_node     = node_load($task_node_nid);
                        $field_view    = field_view_field("node", $task_node,
                          "field_text_task_file");
                        print render($field_view);
                        $stop          = 'Stop';
                        $show          = ditoolsi_training_show_task_answer($task,
                          $account);
                        $lesson_status = ditoolsi_training_get_lesson_status($node,
                          $account);
                        if ($lesson_status == DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED
                          || $lesson_status == DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW
                        ) { // || $current_lesson > $wrapper->field_lesson_number->value()
                          $form = ditoolsi_training_text_task_history($node,
                            $task);
                        }
                        else {
                          $is_curator = mgc_user_has_curator_role();
                          if (!$is_curator) {
                            $form = drupal_get_form('ditoolsi_training_save_task_form',
                              $node, $task);
                          }

                        }
                      }
                      break;
                  }

                  print render($form);
                  print '</div>';
                }
              }
              ?>
        </div>
    </div>
<?php
if (!$is_lesson_finished && $allowed_for_review) {
  print l(t('Send for review'),
    'course/' . $course->nid . '/' . $node->nid . '/to-review', array(
      'attributes' => array(
        'class' => array(
          'btn',
          'btn-default',
          'dit-submit',
          'use-ajax',
          'send-to-review',
        ),
      ),
    ));
}
?>
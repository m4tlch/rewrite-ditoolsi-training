<?php

// Code of error `Invalid excel file`.
define('DITOOLSI_RECRUITMENT_EMAIL_INVALID', 2);

define('DITOOLSI_RECRUITMENT_EMAIL', 'mail');
define('DITOOLSI_DATE_FORMAT', 'd m Y');
define('MGC_REQUESTS_FOR_REVIEW_PER_PAGE', 100);

/**
 * [ditoolsi_training_add_lesson_form description]
 * @param  [type] $form        [description]
 * @param  [type] &$form_state [description]
 * @param  [type] $course      [description]
 * @return [type]              [description]
 */
function ditoolsi_training_add_lesson_form(
  $form,
  &$form_state,
  $course,
  $lesson = NULL
) {
  global $user;
  if (!isset($form_state['storage'])) {
    $form_state['storage'] = array();
  }

  $storage = &$form_state['storage'];

  // This values only for tasks.
  // Values for videos and presentations, defined in fields.
  $default_value = array(
    'task_type' => DITOOLSI_TRAINING_TASK_TYPE_TEXT_TASK,
    'item_id'   => 0,
    'text_task' => array(
      'description'     => '',
      'file'            => NULL,
      'redaction'       => FALSE,
      'first_redaction' => '',
    ),
    'auto_test' => array(
      'title'              => '',
      'freeze_time'        => '',
      'approval_threshold' => '',
      'time_completion'    => '',
      'questions'          => array(
        0 => array(
          'title'   => '',
          'weight'  => '',
          'item_id' => 0,
          'answers' => array(
            0 => array(
              'answer'  => '',
              'correct' => 0,
              'item_id' => 0,
            ),
          ),
        ),
      ),
    ),
  );

  if (!empty($lesson->nid) && !isset($storage['default_values'])) {
    $tasks                     = ditoolsi_training_get_tasks($lesson->nid);
    $tasks                     = array_values($tasks);
    $storage['default_values'] = array();
    $stop                      = 'Stop';
    $storage_values            = &$storage['default_values'];

    foreach ($tasks as $i => $task) {
      $t_wrapper                     = entity_metadata_wrapper('node', $task);
      $storage_values[$i]            = $default_value;
      $storage_values[$i]['item_id'] = $t_wrapper->getIdentifier();

      switch ($task->type) {
        case 'lesson_auto_test':
          $storage_values[$i]['task_type']                       = DITOOLSI_TRAINING_TASK_TYPE_AUTO_TEST;
          $storage_values[$i]['auto_test']['title']              = $t_wrapper->title->value();
          $storage_values[$i]['auto_test']['freeze_time']        = $t_wrapper->field_autotest_freeze_time->value();
          $storage_values[$i]['auto_test']['approval_threshold'] = $t_wrapper->field_autotest_threshold->value();
          $storage_values[$i]['auto_test']['time_completion']    = $t_wrapper->field_autotest_time_completion->value();

          $questions = $t_wrapper->field_autotest_questions->value();

          if ($questions) {
            $storage_values[$i]['auto_test']['questions'] = array();

            foreach ($questions as $iq => $question) {
              $q_wrapper                                         = entity_metadata_wrapper('field_collection_item',
                $question);
              $storage_values[$i]['auto_test']['questions'][$iq] = array(
                'title'   => $q_wrapper->field_autotest_question->value(),
                'weight'  => $q_wrapper->field_autotest_question_weight->value(),
                'item_id' => $q_wrapper->getIdentifier(),
                'answers' => array(
                  0 => array(
                    'answer'  => '',
                    'correct' => 0,
                    'item_id' => 0,
                  ),
                ),
              );

              $answers = $q_wrapper->field_autotest_answers->value();

              if ($answers) {
                $storage_values[$i]['auto_test']['questions'][$iq]['answers'] = array();

                foreach ($answers as $ia => $answer) {
                  $a_wrapper                                                      = entity_metadata_wrapper('field_collection_item',
                    $answer);
                  $storage_values[$i]['auto_test']['questions'][$iq]['answers'][] = array(
                    'answer'  => $a_wrapper->field_question_answer->value(),
                    'correct' => (int) $a_wrapper->field_question_answer_correct->value(),
                    'item_id' => $a_wrapper->getIdentifier(),
                  );
                }
              }
            }
          }
          break;

        case 'lesson_text_task':
          $storage_values[$i]['task_type'] = DITOOLSI_TRAINING_TASK_TYPE_TEXT_TASK;

          if ($body = $t_wrapper->body->value()) {
            $storage_values[$i]['text_task']['description'] = $body['value'];
          }

          if ($redaction = $t_wrapper->field_text_task_redaction->value()) {
            $storage_values[$i]['text_task']['redaction'] = $redaction;
          }

          if ($first_redaction = $t_wrapper->field_text_task_first_redaction->value()) {
            $storage_values[$i]['text_task']['first_redaction'] = $first_redaction['value'];
          }

          if ($file = $t_wrapper->field_text_task_file->value()) {
            $storage_values[$i]['text_task']['file'] = $file['fid'];
          }
          break;
      }
    }
  }
  elseif (!isset($storage['default_values'])) {
    $storage['default_values'] = array(
      $default_value,
    );
  }
  $back_to_course_link      = l(t('Вернуться к списку уроков'),
    '/course/' . $course->nid);
  $form['return_to_course'] = array(
    '#markup' => '<span class="fa fa-arrow-up back-to-course"> ' . $back_to_course_link . '</span>',
  );
  $form['#id']              = 'add-lesson-form';
  $form['#tree']            = TRUE;

  if (empty($lesson)) {
    $lesson = entity_create('node', array(
      'type' => 'lesson',
      'uid'  => $user->uid,
    ));
  }

  $l_wrapper = entity_metadata_wrapper('node', $lesson);

  $file_description = 'Maximum file size: @size. Allowed file types: @formats';
  $format           = array(
    'video'        => 'mp4',
    'audio'        => 'mp3',
    'presentation' => 'ppt pptx pdf',
    'task'         => 'f4v ogv mp4 doc docx pptx ppt jpg png xlsx xls pdf',
  );
  $stop             = 'Stop';
  $audio_file_size  = DITOOLSI_TRAINING_AUDIO_SIZE;
  if (in_array('преподаватель', $user->roles)) {
    $audio_file_size = DITOOLSI_TRAINING_AUDIO_SIZE * 2;
  }
  $description = array(
    'video'        => t($file_description, array(
      '@size'    => t('@size mb', array(
        '@size' => DITOOLSI_TRAINING_VIDEO_SIZE,
      )),
      '@formats' => $format['video'] . '.',
    )),
    'audio'        => t($file_description, array(
      '@size'    => t('@size mb', array(
        '@size' => $audio_file_size,
      )),
      '@formats' => $format['audio'] . '.',
    )),
    'presentation' => t($file_description, array(
      '@size'    => t('@size mb', array(
        '@size' => DITOOLSI_TRAINING_VIDEO_SIZE,
      )),
      '@formats' => $format['presentation'] . '.',
    )),
    'task'         => t($file_description, array(
      '@size'    => t('@size mb', array(
        '@size' => DITOOLSI_TRAINING_VIDEO_SIZE,
      )),
      '@formats' => $format['task'] . '.',
    )),
  );

  $form['info'] = array(
    '#type'  => 'fieldset',
    '#title' => t('Information about lesson'),
    '#tree'  => TRUE,
  );

  $form['info']['number']       = array(
    '#type'          => 'textfield',
    '#title'         => t('Number lesson'),
    '#prefix'        => '<div class="process-ditools-labels number-field">',
    '#suffix'        => '</div>',
    '#default_value' => $l_wrapper->field_lesson_number->value(),
  );
  $stop                         = 'Stop';
  $form['info']['title']        = array(
    '#type'          => 'textfield',
    '#title'         => t('Title of lesson (no more than 120 symbols)'),
    '#prefix'        => '<div class="process-ditools-labels title-field">',
    '#suffix'        => '</div>',
    '#default_value' => $l_wrapper->title->value(),
  );
  $stop                         = 'Stop';
  $form['info']['link_post']    = array(
    '#type'          => 'textfield',
    '#title'         => t('Link for posting'),
    '#prefix'        => '<div class="process-ditools-labels link-post-field">',
    '#suffix'        => '</div>',
    '#default_value' => $l_wrapper->field_lesson_link_post->value(),
  );
  $stop                         = 'Stop';
  $form['info']['message_post'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Message for posting (no more than 120 symbols)'),
    '#prefix'        => '<div class="process-ditools-labels message-post-field">',
    '#suffix'        => '</div>',
    '#default_value' => $l_wrapper->field_lesson_message_post->value(),
  );
  $stop                         = 'Stop';
  $form['info']['check_time']   = array(
    '#type'          => 'textfield',
    '#title'         => 'Время для проверки урока (дней)',
    '#default_value' => $l_wrapper->field_lesson_check_time->value(),
    '#prefix'        => '<div class="process-ditools-labels check-time-field">',
    '#suffix'        => '</div>',
  );
  $stop                         = 'Stop';
  $form['info']['pass_time']    = array(
    '#type'          => 'textfield',
    '#title'         => 'Время для прохождения урока (дней)',
    '#default_value' => $l_wrapper->field_lesson_pass_time->value(),
    '#prefix'        => '<div class="process-ditools-labels pass-time-field">',
    '#suffix'        => '</div>',
  );

  $body = $l_wrapper->body->value();

  $form['info']['body']      = array(
    '#type'          => 'text_format',
    '#format'        => 'filtered_html',
    '#default_value' => isset($body['value']) ? $body['value'] : '',
  );
  $stop                      = 'Stop';
  $form['info']['is_final']  = array(
    '#type'          => 'checkbox',
    '#title'         => t('This is the final lesson'),
    '#return_value'  => 1,
    '#default_value' => $l_wrapper->field_lesson_final->value(),
  );
  $stop                      = 'Stop';
  $form['info']['autocheck'] = array(
    '#type'  => 'fieldset',
    '#title' => t('Настройки автопроверки урока'),
    '#tree'  => TRUE,
  );
  $stop                      = 'Stop';
  try {
    $autocheck_value    = $l_wrapper->field_lesson_check_type->value();
    $low_score          = $l_wrapper->field_low_score->value();
    $high_score         = $l_wrapper->field_high_score->value();
    $autocomments_value = $l_wrapper->field_autocomment->value->value();
  } catch (Exception $e) {

  }
  $autocheck_value                        = (isset($autocheck_value)) ? $autocheck_value : 0;
  $low_score                              = (isset($low_score)) ? $low_score : 0;
  $high_score                             = (isset($high_score)) ? $high_score : 5;
  $autocomments_value                     = (isset($autocomments_value)) ? $autocomments_value : '';
  $form['info']['autocheck']['switcher']  = array(
    '#type'          => 'checkbox',
    '#title'         => t('Этот урок должен проходить автопроверку'),
    '#return_value'  => 1,
    '#default_value' => $autocheck_value,
  );
  $stop                                   = 'Stop';
  $form['info']['autocheck']['low-score'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Нижний порог оценки'),
    '#prefix'        => '<div class="process-ditools-labels number-field">',
    '#suffix'        => '</div>',
    '#description'   => t('Величина, которая будет взята за нижний порог при рандомизации диапазона оценок(от 0 до 5)'),
    '#default_value' => $low_score,
  );

  $form['info']['autocheck']['high-score'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Верхний порог оценки'),
    '#prefix'        => '<div class="process-ditools-labels number-field">',
    '#suffix'        => '</div>',
    '#description'   => t('Величина, которая будет взята за верхний порог при рандомизации диапазона оценок(от 0 до 5)'),
    '#default_value' => $high_score,
  );
  $stop                                    = 'Stop';

  if (isset($lesson->is_new)) {
    $autocomments = '';
  }
  else {
    $autocomments = $autocomments_value;
  }
  $stop = 'Stop';

  $form['info']['autocheck']['autocomments'] = array(
    '#type'          => 'text_format',
    '#format'        => 'plain_text',
    '#description'   => t('Введите список автокомментариев. Каждый комментарий должен располагаться отдельно на новой строке.'),
    '#default_value' => $autocomments,
  );

  $form['attach'] = array(
    '#type'  => 'fieldset',
    '#title' => t('Attach to the lesson'),
    '#tree'  => TRUE,
  );

  $form['attach']['tabs']['switcher'] = array(
    '#theme'      => 'item_list',
    '#type'       => 'ul',
    '#prefix'     => '<div id="lesson-attach-content">',
    '#items'      => array(
      '<a href="#to-video">' . t('Video') . '</a>',
      '<a href="#to-audio">' . t('Audio') . '</a>',
      '<a href="#to-presentation">' . t('Presentation') . '</a>',
      '<a href="#to-task">' . t('Task') . '</a>',
    ),
    '#attributes' => array(
      'id' => 'lesson-attach-tabs',
    ),
  );

  $form['attach']['tabs']['content']['video'] = array(
    '#type' => 'container',
    '#id'   => 'to-video',
  );

  $form['attach']['tabs']['content']['video']['switcher'] = array(
    '#theme'      => 'item_list',
    '#type'       => 'ul',
    '#items'      => array(
      '<a href="#to-video-0">1</a>',
      '<a href="#to-video-1">2</a>',
      '<a href="#to-video-2">3</a>',
      '<a href="#to-video-3">4</a>',
    ),
    '#attributes' => array(
      'id'    => 'video-switcher-tabs',
      'class' => array(
        'clearfix',
      ),
    ),
  );

  $fc_video = $l_wrapper->field_lesson_video->value();

  for ($i = 0; $i <= 3; $i++) {
    if (!isset($fc_video[$i])) {
      $fc_video[$i] = entity_create('field_collection_item', array(
        'field_name' => 'field_lesson_video',
      ));
    }

    $v_wrapper = entity_metadata_wrapper('field_collection_item',
      $fc_video[$i]);

    $form['attach']['tabs']['content']['video']['content'][$i] = array(
      '#type' => 'container',
      '#id'   => 'to-video-' . $i,
    );

    $container          = &$form['attach']['tabs']['content']['video']['content'][$i];
    $container['title'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Title'),
      '#default_value' => $v_wrapper->field_lesson_video_title->value(),
    );

    $btn = '<button type="button" target="upload-lesson-video-file-' . $i . '" class="btn btn-success add-button">'
      . '<i class="fa fa-plus-circle"></i> ' . t('Add') . '</button>';


    $container['file'] = array(
      '#type'              => 'managed_file',
      '#upload_location'   => 'public://lessons/video',
      '#title'             => t('Select a file'),
      '#id'                => 'upload-lesson-video-file-' . $i,
      '#field_suffix'      => $btn . '<div class="file-description">' . $description['video'] . '</div>',
      '#upload_validators' => array(
        'file_validate_extensions' => array($format['video']),
        'file_validate_size'       => array(DITOOLSI_TRAINING_VIDEO_SIZE * 1024 * 1024),
      ),
    );

    if ($file = $v_wrapper->field_lesson_video_file->value()) {
      $container['file']['#default_value'] = $file['fid'];
    }

    $btn = '<button type="button" target="upload-lesson-video-url-' . $i . '" class="btn btn-success add-button">'
      . '<i class="fa fa-plus-circle"></i> ' . t('Add') . '</button>';

    $container['url']           = array(
      '#type'          => 'textfield',
      '#title'         => t('Or add link') . ':',
      '#field_suffix'  => $btn,
      '#default_value' => $v_wrapper->field_lesson_video_url->value(),
    );
    $container['embedded_code'] = array(
      '#type'          => 'textarea',
      '#title'         => t('Код плеера') . ':',
      //'#field_suffix'  => $btn,
      '#default_value' => $v_wrapper->field_embedded_code->value(),
    );

    $container['item_id'] = array(
      '#type'  => 'hidden',
      '#value' => (int) $v_wrapper->getIdentifier(),
    );
  }

  $form['attach']['tabs']['content']['audio'] = array(
    '#type' => 'container',
    '#id'   => 'to-audio',
  );

  $form['attach']['tabs']['content']['audio']['switcher'] = array(
    '#theme'      => 'item_list',
    '#type'       => 'ul',
    '#items'      => array(
      '<a href="#to-audio-0">1</a>',
      '<a href="#to-audio-1">2</a>',
      '<a href="#to-audio-2">3</a>',
      '<a href="#to-audio-3">4</a>',
    ),
    '#attributes' => array(
      'id'    => 'audio-switcher-tabs',
      'class' => array(
        'clearfix',
      ),
    ),
  );

  $fc_audio = $l_wrapper->field_lesson_audio->value();

  for ($i = 0; $i <= 3; $i++) {
    if (!isset($fc_audio[$i])) {
      $fc_audio[$i] = entity_create('field_collection_item', array(
        'field_name' => 'field_lesson_audio',
      ));
    }

    $v_wrapper = entity_metadata_wrapper('field_collection_item',
      $fc_audio[$i]);

    $form['attach']['tabs']['content']['audio']['content'][$i] = array(
      '#type' => 'container',
      '#id'   => 'to-audio-' . $i,
    );

    $container          = &$form['attach']['tabs']['content']['audio']['content'][$i];
    $container['title'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Title'),
      '#default_value' => $v_wrapper->field_lesson_audio_title->value(),
    );

    $btn = '<button type="button" target="upload-lesson-audio-file-' . $i . '" class="btn btn-success add-button">'
      . '<i class="fa fa-plus-circle"></i> ' . t('Add') . '</button>';

    $container['file'] = array(
      '#type'              => 'managed_file',
      '#upload_location'   => 'public://lessons/audio',
      '#title'             => t('Select a file'),
      '#id'                => 'upload-lesson-audio-file-' . $i,
      '#field_suffix'      => $btn . '<div class="file-description">' . $description['audio'] . '</div>',
      '#upload_validators' => array(
        'file_validate_extensions' => array($format['audio']),
        'file_validate_size'       => array(DITOOLSI_TRAINING_AUDIO_SIZE * 1024 * 1024),
      ),
    );

    if ($file = $v_wrapper->field_lesson_audio_file->value()) {
      $container['file']['#default_value'] = $file['fid'];
    }

    $container['item_id'] = array(
      '#type'  => 'hidden',
      '#value' => (int) $v_wrapper->getIdentifier(),
    );
  }

  $form['attach']['tabs']['content']['presentation'] = array(
    '#type' => 'container',
    '#id'   => 'to-presentation',
  );

  $form['attach']['tabs']['content']['presentation']['switcher'] = array(
    '#theme'      => 'item_list',
    '#type'       => 'ul',
    '#items'      => array(
      '<a href="#to-presentation-0">1</a>',
      '<a href="#to-presentation-1">2</a>',
      '<a href="#to-presentation-2">3</a>',
      '<a href="#to-presentation-3">4</a>',
    ),
    '#attributes' => array(
      'id'    => 'presentation-switcher-tabs',
      'class' => array(
        'clearfix',
      ),
    ),
  );

  for ($i = 0; $i <= 3; $i++) {
    $fc_presentation = $l_wrapper->field_lesson_presentation->value();

    if (!isset($fc_presentation[$i])) {
      $fc_presentation[$i] = entity_create('field_collection_item', array(
        'field_name' => 'field_lesson_presentation',
      ));
    }

    $p_wrapper = entity_metadata_wrapper('field_collection_item',
      $fc_presentation[$i]);

    $form['attach']['tabs']['content']['presentation']['content'][$i] = array(
      '#type' => 'container',
      '#id'   => 'to-presentation-' . $i,
    );

    $container          = &$form['attach']['tabs']['content']['presentation']['content'][$i];
    $container['title'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Title'),
      '#default_value' => $p_wrapper->field_lesson_presentation_title->value(),
    );

    $btn = '<button type="button" target="upload-lesson-presentation-file-' . $i . '" class="btn btn-success add-button">'
      . '<i class="fa fa-plus-circle"></i> ' . t('Add') . '</button>';

    $container['file'] = array(
      '#type'              => 'managed_file',
      '#upload_location'   => 'public://lessons/presentation',
      '#title'             => t('Select a file'),
      '#id'                => 'upload-lesson-presentation-file-' . $i,
      '#field_suffix'      => $btn . '<div class="file-description">' . $description['presentation'] . '</div>',
      '#upload_validators' => array(
        'file_validate_extensions' => array($format['presentation']),
        'file_validate_size'       => array(DITOOLSI_TRAINING_VIDEO_SIZE * 1024 * 1024),
      ),
    );

    if ($file = $p_wrapper->field_lesson_presentation_file->value()) {
      $container['file']['#default_value'] = $file['fid'];
    }

    $container['item_id'] = array(
      '#type'  => 'hidden',
      '#value' => $p_wrapper->getIdentifier(),
    );
  }

  $form['attach']['tabs']['content']['task'] = array(
    '#type'   => 'container',
    '#id'     => 'to-task',
    '#suffix' => '</div>',
    // Opened in prefix in $form['attach']['tabs']['switcher']
  );

  $links_to_task = array();

  $i = 0;

  foreach ($storage['default_values'] as $key => $value) {
    $links_to_task[] = '<a href="#to-task-' . $i . '">' . (++$i) . '</a>';
  }

  $form['attach']['tabs']['content']['task']['switcher'] = array(
    '#theme'      => 'item_list',
    '#type'       => 'ul',
    '#items'      => $links_to_task,
    '#attributes' => array(
      'id'    => 'task-switcher-tabs',
      'class' => array(
        'clearfix',
      ),
    ),
  );

  $form['attach']['tabs']['content']['task']['add_task'] = array(
    '#type'                    => 'submit',
    '#value'                   => t('Add task'),
    '#submit'                  => array('ditoolsi_training_add_task_submit'),
    '#ajax'                    => array(
      'callback' => 'ditoolsi_training_task_ajax_callback',
      'wrapper'  => 'to-task',
      'event'    => 'click',
    ),
    '#limit_validation_errors' => array(),
  );

  $storage_values = $storage['default_values'];

  $j = 0;

  foreach ($storage['default_values'] as $i => $value) {
    $form['attach']['tabs']['content']['task']['content'][$i] = array(
      '#type' => 'container',
      '#id'   => 'to-task-' . $j,
    );

    $container = &$form['attach']['tabs']['content']['task']['content'][$i];

    $container['task_type'] = array(
      '#type'          => 'select',
      '#options'       => array(
        DITOOLSI_TRAINING_TASK_TYPE_AUTO_TEST => t('Auto test'),
        DITOOLSI_TRAINING_TASK_TYPE_TEXT_TASK => t('Text task'),
      ),
      '#attributes'    => array(
        'class' => array(
          'task-type-select',
        ),
      ),
      '#prefix'        => '<div class="select-task-type">',
      '#suffix'        => '</div>',
      '#default_value' => $storage_values[$i]['task_type'],
    );

    $container['text_task'] = array(
      '#type'   => 'container',
      '#states' => array(
        'visible' => array(
          ':input[name="attach[tabs][content][task][content][' . $i . '][task_type]"]' => array(
            'value' => DITOOLSI_TRAINING_TASK_TYPE_TEXT_TASK,
          ),
        ),
      ),
    );

    $container['text_task']['description'] = array(
      '#type'          => 'text_format',
      '#format'        => 'filtered_html',
      '#default_value' => $storage_values[$i]['text_task']['description'],
    );

    $container['text_task']['redaction'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Task with redaction'),
      '#default_value' => $storage_values[$i]['text_task']['redaction'],
    );

    $container['text_task']['first_redaction'] = array(
      '#type'          => 'text_format',
      '#format'        => 'filtered_html',
      '#default_value' => $storage_values[$i]['text_task']['first_redaction'],
      '#states'        => array(
        'invisible' => array(
          ':input[name="attach[tabs][content][task][content][' . $i . '][text_task][redaction]"]' => array('checked' => FALSE),
        ),
      ),
    );

    $btn = '<button type="button" target="upload-lesson-task-file-' . $i . '" class="btn btn-success add-button">'
      . '<i class="fa fa-plus-circle"></i> ' . t('Add') . '</button>';

    $container['text_task']['file'] = array(
      '#type'              => 'managed_file',
      '#upload_location'   => 'public://lessons/task/text',
      '#title'             => t('Select a file'),
      '#id'                => 'upload-lesson-task-file-' . $i,
      '#field_suffix'      => $btn . '<div class="file-description">' . $description['task'] . '</div>',
      '#default_value'     => $storage_values[$i]['text_task']['file'],
      '#upload_validators' => array(
        'file_validate_extensions' => array($format['task']),
        'file_validate_size'       => array(DITOOLSI_TRAINING_VIDEO_SIZE * 1024 * 1024),
      ),
    );

    $container['auto_test'] = array(
      '#type'   => 'container',
      '#prefix' => '<div class="process-ditools-labels auto-test-list" id="auto-test-' . $i . '">',
      '#suffix' => '</div>',
      '#states' => array(
        'visible' => array(
          ':input[name="attach[tabs][content][task][content][' . $i . '][task_type]"]' => array(
            'value' => DITOOLSI_TRAINING_TASK_TYPE_AUTO_TEST,
          ),
        ),
      ),
    );

    $container['auto_test']['title'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Title of test (no more than 120 symbols)'),
      '#default_value' => $storage_values[$i]['auto_test']['title'],
    );

    $container['auto_test']['freeze_time'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Freeze time (from 1 to 100 min.)'),
      '#prefix'        => '<div class="row three-fields-line"><div class="col-xs-4 col">',
      '#suffix'        => '</div>',
      '#default_value' => $storage_values[$i]['auto_test']['freeze_time'],
    );

    $container['auto_test']['approval_threshold'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Approval threshold (%)'),
      '#prefix'        => '<div class="col-xs-4 col">',
      '#suffix'        => '</div>',
      '#default_value' => $storage_values[$i]['auto_test']['approval_threshold'],
    );

    $container['auto_test']['time_completion'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Time for completion (from 1 to 100 min.)'),
      '#prefix'        => '<div class="col-xs-4 col">',
      '#suffix'        => '</div></div>',
      '#default_value' => $storage_values[$i]['auto_test']['time_completion'],
    );

    // Questions.
    $container['auto_test']['questions'] = array(
      '#type'       => 'container',
      '#attributes' => array(
        'class' => array(
          'list-questions',
        ),
      ),
    );

    $questions = &$container['auto_test']['questions'];

    if (!isset($storage['auto_test']['questions'][$i])) {
      $storage['auto_test']['questions'][$i] = array(array(0));
    }

    $ir = 0;

    foreach ($storage_values[$i]['auto_test']['questions'] as $iq => $question) {
      if (!isset($form_state['storage']['auto_test']['questions'][$i][$iq])) {
        $form_state['storage']['auto_test']['questions'][$i][$iq] = array();
      }

      foreach ($question['answers'] as $ia => $answer) {
        if (!isset($form_state['storage']['auto_test']['questions'][$i][$iq][$ia])) {
          $form_state['storage']['auto_test']['questions'][$i][$iq][$ia] = 0;
        }
      }
    }

    foreach ($form_state['storage']['auto_test']['questions'][$i] as $iq => $ans) {
      $ir++;

      if (!isset($storage['auto_test']['questions'][$i][$iq])) {
        $storage['auto_test']['questions'][$i][$iq] = array(0);
        $ans                                        = $storage['auto_test']['questions'][$i][$iq];
      }

      $questions[$iq] = array(
        '#type'       => 'fieldset',
        '#title'      => t('Question @i', array(
          '@i' => $ir,
        )),
        '#attributes' => array(
          'class' => array(
            'question-fieldset',
          ),
        ),
      );

      $questions[$iq]['remove'] = array(
        '#type'                    => 'submit',
        '#value'                   => t('Remove question'),
        '#name'                    => 'remove_question[' . $i . '][' . $iq . ']',
        '#submit'                  => array('ditoolsi_training_remove_question_submit'),
        '#attributes'              => array(
          'class' => array(
            'remove-link',
            'remove-link-question',
          ),
        ),
        '#limit_validation_errors' => array(),
        '#ajax'                    => array(
          'callback' => 'ditoolsi_training_remove_question_ajax',
          'keypress' => TRUE,
          'event'    => 'click',
          'wrapper'  => 'auto-test-' . $i,
        ),
      );

      $questions[$iq]['title'] = array(
        '#type'          => 'textarea',
        '#title'         => t('Question'),
        // '#maxlength' => 60,
        '#default_value' => $storage_values[$i]['auto_test']['questions'][$iq]['title'],
      );

      $questions[$iq]['item_id'] = array(
        '#type'  => 'hidden',
        '#value' => $storage_values[$i]['auto_test']['questions'][$iq]['item_id'],
      );

      $questions[$iq]['answers'] = array(
        '#type'       => 'container',
        '#prefix'     => '<div id="auto-test-' . $i . '-answers-' . $iq . '">',
        '#suffix'     => '</div>',
        '#attributes' => array(
          'class' => array(
            'answer-container',
          ),
        ),
      );

      $answers = &$questions[$iq]['answers'];

      foreach ($ans as $ia => $value) {
        $answers[$ia]['answer'] = array(
          '#type'          => 'textarea',
          '#title'         => t('Variant of answer'),
          '#prefix'        => '<div class="answer-fields clearfix"><div class="answer-field">',
          '#suffix'        => '</div>',
          '#default_value' => $storage_values[$i]['auto_test']['questions'][$iq]['answers'][$ia]['answer'],
        );

        $answers[$ia]['correct'] = array(
          '#type'          => 'checkbox',
          '#title'         => t('The correct answer'),
          '#prefix'        => '<div class="correct-field">',
          '#suffix'        => '</div>',
          '#default_value' => $storage_values[$i]['auto_test']['questions'][$iq]['answers'][$ia]['correct'],
          '#attributes'    => array(
            'class' => array(
              'correct-answer-checkbox',
            ),
          ),
        );

        $answers[$ia]['item_id'] = array(
          '#type'  => 'hidden',
          '#value' => $storage_values[$i]['auto_test']['questions'][$iq]['answers'][$ia]['item_id'],
        );

        $answers[$ia]['remove'] = array(
          '#type'       => 'submit',
          '#value'      => t('Remove answer'),
          '#name'       => 'remove_question[' . $i . '][' . $iq . '][' . $ia . ']',
          '#submit'     => array('ditoolsi_training_remove_answer_submit'),
          '#prefix'     => '<div class="remove-answer-button">',
          '#suffix'     => '</div></div>',
          // Second div opened for prefix in $answers[ $ia ]['answer']
          '#attributes' => array(
            'class' => array(
              'remove-link',
              'remove-link-answer',
            ),
          ),
          '#ajax'       => array(
            'callback' => 'ditoolsi_training_remove_answer_ajax',
            'keypress' => TRUE,
            'event'    => 'click',
            'wrapper'  => 'auto-test-' . $i . '-answers-' . $iq,
          ),
        );
      }

      $questions[$iq]['add_answer'] = array(
        '#type'                    => 'submit',
        '#value'                   => t('Add variant of answer'),
        '#name'                    => 'add_answer[' . $i . '][' . $iq . ']',
        '#submit'                  => array('ditoolsi_training_add_answer_submit'),
        '#attributes'              => array(
          'class' => array(
            'add-answer-button',
          ),
        ),
        '#limit_validation_errors' => array(),
        '#ajax'                    => array(
          'callback' => 'ditoolsi_training_add_answer_ajax',
          'keypress' => TRUE,
          'wrapper'  => 'auto-test-' . $i . '-answers-' . $iq,
          'event'    => 'click',
        ),
      );

      $questions[$iq]['weight'] = array(
        '#type'          => 'textfield',
        '#title'         => t('Weight of question (%)'),
        '#prefix'        => '<div class="weight-field">',
        '#suffix'        => '</div>',
        '#default_value' => $storage_values[$i]['auto_test']['questions'][$iq]['weight'],
      );
    }

    $container['auto_test']['add_question'] = array(
      '#type'                    => 'submit',
      '#value'                   => t('Add question'),
      '#name'                    => 'add_question[' . $i . ']',
      '#submit'                  => array('ditoolsi_training_add_question_submit'),
      '#attributes'              => array(
        'class' => array(
          'add-button',
          'add-question-button',
        ),
      ),
      '#limit_validation_errors' => array(),
      '#ajax'                    => array(
        'callback' => 'ditoolsi_training_add_question_ajax',
        'keypress' => TRUE,
        'wrapper'  => 'auto-test-' . $i,
        'event'    => 'click',
      ),
    );

    $container['remove_task'] = array(
      '#type'                    => 'submit',
      '#value'                   => t('Delete task'),
      '#name'                    => 'remove_task[' . $i . ']',
      '#submit'                  => array('ditoolsi_training_remove_task_submit'),
      '#attributes'              => array(
        'class' => array(
          'remove-button',
        ),
      ),
      '#limit_validation_errors' => array(),
      '#ajax'                    => array(
        'callback' => 'ditoolsi_training_remove_task_ajax',
        'wrapper'  => 'to-task',
        'event'    => 'click',
      ),
    );

    $container['item_id'] = array(
      '#type'  => 'hidden',
      '#value' => $storage_values[$i]['item_id'],
    );

    $j++;
  }

  $form['actions'] = array(
    '#type' => 'actions',
  );

  $form['actions']['submit'] = array(
    '#type'       => 'submit',
    '#value'      => t('Save lesson'),
    '#attributes' => array(
      'class' => array(
        'dit-submit',
        'save-lesson-submit',
      ),
    ),
    // '#ajax' => array(
    //   'callback' => 'ditoolsi_training_add_lesson_form_ajax',
    //   'event'    => 'click',
    //   'wrapper'  => 'add-lesson-form',
    // ),
  );

  if (!empty($lesson->nid)) {
    $form['actions']['delete'] = array(
      '#type'       => 'submit',
      '#value'      => t('Delete lesson'),
      '#attributes' => array(
        // 'class' => array(
        //   'dit-submit',
        //   'delete-lesson-submit',
        // ),
      ),
      '#submit'     => array('ditoolsi_training_redirect_after_delete_lesson'),
    );
  }

  $form['#after_build'][] = 'ditoolsi_training_add_lesson_form_after_build';

  $form['#attached']['css'] = array(
    drupal_get_path('theme', 'bootstrap_subtheme') . '/css/add-lesson.css',
    drupal_get_path('theme',
      'bootstrap_subtheme') . '/css/jquery/jquery.sliderTabs.min.css',
  );

  $form['#attached']['js'] = array(
    drupal_get_path('module', 'ditoolsi_training') . '/js/add-lesson.js',
    drupal_get_path('theme',
      'bootstrap_subtheme') . '/js/jquery/jquery.sliderTabs.js',
  );

  return $form;
}

/**
 * [ditoolsi_training_add_lesson_form_after_build description]
 * @param  [type] $form        [description]
 * @param  [type] &$form_state [description]
 * @return [type]              [description]
 */
function ditoolsi_training_add_lesson_form_after_build($form, &$form_state) {
  $form['info']['body']['format']['#access'] = FALSE;

  $form['info']['autocheck']['autocomments'] ['format']['#access'] = FALSE;
  if (isset($form['attach']['tabs']['content']['task']['content'])) {
    foreach ($form['attach']['tabs']['content']['task']['content'] as &$container) {
      if (isset($container['text_task']['description'])) {
        $container['text_task']['description']['format']['#access'] = FALSE;
      }
      if (isset($container['text_task']['first_redaction'])) {
        $container['text_task']['first_redaction']['format']['#access'] = FALSE;
      }
    }
  }

  return $form;
}

/**
 * [ditoolsi_training_task_ajax_callback description]
 * @param  [type] $form       [description]
 * @param  [type] $form_state [description]
 * @return [type]             [description]
 */
function ditoolsi_training_task_ajax_callback($form, $form_state) {
  return $form['attach']['tabs']['content']['task'];
}

/**
 * [ditoolsi_training_add_task_submit description]
 * @param  [type] $form        [description]
 * @param  [type] &$form_state [description]
 * @return [type]              [description]
 */
function ditoolsi_training_add_task_submit($form, &$form_state) {
  $default_value = array(
    'task_type' => DITOOLSI_TRAINING_TASK_TYPE_TEXT_TASK,
    'item_id'   => 0,
    'text_task' => array(
      'description'     => '',
      'file'            => NULL,
      'redaction'       => FALSE,
      'first_redaction' => '',
    ),
    'auto_test' => array(
      'title'              => '',
      'freeze_time'        => '',
      'approval_threshold' => '',
      'time_completion'    => '',
      'questions'          => array(
        0 => array(
          'title'   => '',
          'weight'  => '',
          'item_id' => 0,
          'answers' => array(
            0 => array(
              'answer'  => '',
              'correct' => 0,
              'item_id' => 0,
            ),
          ),
        ),
      ),
    ),
  );

  $form_state['storage']['default_values'][] = $default_value;
  $form_state['rebuild']                     = TRUE;
}

/**
 * [ditoolsi_training_remove_task_ajax description]
 * @param  [type] $form       [description]
 * @param  [type] $form_state [description]
 * @return [type]             [description]
 */
function ditoolsi_training_remove_task_ajax($form, $form_state) {
  return $form['attach']['tabs']['content']['task'];
}

/**
 * [ditoolsi_training_remove_task_submit description]
 * @param  [type] $form        [description]
 * @param  [type] &$form_state [description]
 * @return [type]              [description]
 */
function ditoolsi_training_remove_task_submit($form, &$form_state) {
  if (count($form_state['storage']['default_values']) > 1) {
    $element              = $form_state['triggering_element'];
    $element_container_id = $element['#parents'][5];
    if (isset($form_state['storage']['default_values'][$element_container_id])) {
      if (!empty($form_state['storage']['default_values'][$element_container_id]['item_id'])) {
        $form_state['storage']['task_to_remove'][] = $form_state['storage']['default_values'][$element_container_id]['item_id'];
      }
      unset($form_state['storage']['default_values'][$element_container_id]);
    }
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * [ditoolsi_training_add_question_page description]
 * @param  [type] $course [description]
 * @return [type]         [description]
 */
function ditoolsi_training_add_question_ajax($form, $form_state) {
  $element    = $form_state['triggering_element'];
  $element_id = $element['#parents'][5];

  return $form['attach']['tabs']['content']['task']['content'][$element_id]['auto_test'];
}

/**
 * [ditoolsi_training_add_question_submit description]
 * @param  [type] $form        [description]
 * @param  [type] &$form_state [description]
 * @return [type]              [description]
 */
function ditoolsi_training_add_question_submit($form, &$form_state) {
  $element    = $form_state['triggering_element'];
  $element_id = $element['#parents'][5];

  $form_state['storage']['auto_test']['questions'][$element_id][] = array(0);

  $keys = array_keys($form_state['storage']['auto_test']['questions'][$element_id]);
  $key  = end($keys);

  $form_state['storage']['default_values'][$element_id]['auto_test']['questions'][$key] = array(
    'title'   => '',
    'weight'  => '',
    'item_id' => 0,
    'answers' => array(
      0 => array(
        'answer'  => '',
        'correct' => 0,
        'item_id' => 0,
      ),
    ),
  );

  $form_state['rebuild'] = TRUE;
}

/**
 * [ditoolsi_training_add_answer_ajax description]
 * @param  [type] $form       [description]
 * @param  [type] $form_state [description]
 * @return [type]             [description]
 */
function ditoolsi_training_add_answer_ajax($form, $form_state) {
  $element           = $form_state['triggering_element'];
  $element_parent_id = $element['#parents'][5];
  $element_id        = $element['#parents'][8];

  return $form['attach']['tabs']['content']['task']['content'][$element_parent_id]['auto_test']['questions'][$element_id]['answers'];
}

/**
 * [ditoolsi_training_add_answer_submit description]
 * @param  [type] $form        [description]
 * @param  [type] &$form_state [description]
 * @return [type]              [description]
 */
function ditoolsi_training_add_answer_submit($form, &$form_state) {
  $element           = $form_state['triggering_element'];
  $element_parent_id = $element['#parents'][5];
  $element_id        = $element['#parents'][8];

  $form_state['storage']['auto_test']['questions'][$element_parent_id][$element_id][] = 0;

  $keys = array_keys($form_state['storage']['auto_test']['questions'][$element_parent_id][$element_id]);
  $key  = end($keys);

  $form_state['storage']['default_values'][$element_parent_id]['auto_test']['questions'][$element_id]['answers'][$key] = array(
    'answer'  => '',
    'correct' => '',
    'item_id' => 0,
  );

  $form_state['rebuild'] = TRUE;
}

/**
 * [ditoolsi_training_remove_question_ajax description]
 * @param  [type] $form       [description]
 * @param  [type] $form_state [description]
 * @return [type]             [description]
 */
function ditoolsi_training_remove_question_ajax($form, $form_state) {
  $element    = $form_state['triggering_element'];
  $element_id = $element['#parents'][5];

  return $form['attach']['tabs']['content']['task']['content'][$element_id]['auto_test'];
}

/**
 * [ditoolsi_training_remove_question_submit description]
 * @param  [type] $form        [description]
 * @param  [type] &$form_state [description]
 * @return [type]              [description]
 */
function ditoolsi_training_remove_question_submit($form, &$form_state) {
  $element           = $form_state['triggering_element'];
  $element_parent_id = $element['#parents'][5];
  $element_id        = $element['#parents'][8];

  if (isset($form_state['storage']['auto_test']['questions'][$element_parent_id][$element_id])) {
    unset($form_state['storage']['auto_test']['questions'][$element_parent_id][$element_id]);
    unset($form_state['storage']['default_values'][$element_parent_id]['auto_test']['questions'][$element_id]);
  }

  $form_state['rebuild'] = TRUE;
}

/**
 * [ditoolsi_training_remove_answer_ajax description]
 * @param  [type] $form       [description]
 * @param  [type] $form_state [description]
 * @return [type]             [description]
 */
function ditoolsi_training_remove_answer_ajax($form, $form_state) {
  $element           = $form_state['triggering_element'];
  $element_parent_id = $element['#parents'][5];
  $element_id        = $element['#parents'][8];

  return $form['attach']['tabs']['content']['task']['content'][$element_parent_id]['auto_test']['questions'][$element_id]['answers'];
}

/**
 * [ditoolsi_training_remove_answer_submit description]
 * @param  [type] $form        [description]
 * @param  [type] &$form_state [description]
 * @return [type]              [description]
 */
function ditoolsi_training_remove_answer_submit($form, &$form_state) {
  $element              = $form_state['triggering_element'];
  $element_container_id = $element['#parents'][5];
  $element_parent_id    = $element['#parents'][8];
  $element_id           = $element['#parents'][10];

  $storage_questions = &$form_state['storage']['auto_test']['questions'];

  if (isset($storage_questions[$element_container_id][$element_parent_id][$element_id])) {
    unset($storage_questions[$element_container_id][$element_parent_id][$element_id]);
    unset($form_state['storage']['default_values'][$element_container_id]['auto_test']['questions'][$element_parent_id]['answers'][$element_id]);
  }

  $form_state['rebuild'] = TRUE;
}

/**
 * [ditoolsi_training_add_lesson_form_ajax description]
 * @param  [type] $form       [description]
 * @param  [type] $form_state [description]
 * @return [type]             [description]
 */
function ditoolsi_training_add_lesson_form_ajax($form, $form_state) {
  return $form;
}

function ditoolsi_training_add_lesson_form_validate($form, &$form_state) {
  $values       = $form_state['values'];
  $info         = $values['info'];
  $attach       = $values['attach']['tabs']['content'];
  $video        = $attach['video']['content'];
  $presentation = $attach['presentation']['content'];
  $tasks        = $attach['task']['content'];
  $course       = $form_state['build_info']['args'][0];
  $lesson       = isset($form_state['build_info']['args'][1]) ? $form_state['build_info']['args'][1] : FALSE;

  if ($lesson && $field_lesson_number = field_get_items('node', $lesson,
      'field_lesson_number')
  ) {
    $current_number = $field_lesson_number[0]['value'];
  }
  else {
    $current_number = 0;
  }

  // Validate lesson number.
  $number = trim($info['number']);
  if (empty($number)) {
    form_set_error('info][number', t('!name field is required.', array(
      '!name' => t('Number lesson'),
    )));
  }
  elseif (!is_numeric($number) || intval($number) != $number || $number <= 0) {
    form_set_error('info][number', t('!name must be a positive integer.', array(
      '!name' => t('Number lesson'),
    )));
  }

  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'lesson')
    ->propertyCondition('status', NODE_PUBLISHED)
    ->fieldCondition('field_lesson_course', 'target_id', $course->nid)
    ->fieldCondition('field_lesson_number', 'value', $number)
    ->fieldCondition('field_lesson_number', 'value', $current_number, '<>');

  $result = $query->execute();

  if (isset($result['node'])) {
    form_set_error('info][number',
      t('The lesson with this number already exists.'));
  }

  // Validate title of lesson.
  $title = trim($info['title']);

  if (empty($title)) {
    form_set_error('info][title', t('!name field is required.', array(
      '!name' => t('Title of lesson'),
    )));
  }

  $tasks_filled = 0;

  foreach ($tasks as $task_key => $task) {
    switch ($task['task_type']) {
      case DITOOLSI_TRAINING_TASK_TYPE_AUTO_TEST:
        $questions    = 0;
        $total_weight = 0;
        if (!empty($task['auto_test']['questions'])) {
          foreach ($task['auto_test']['questions'] as $question_key => $question) {
            if ($question['title']) {
              $answers         = 0;
              $answers_correct = 0;
              if (!empty($question['answers'])) {
                foreach ($question['answers'] as $answer_key => $answer) {
                  if ($answer['answer']) {
                    $answers++;
                  }
                  if ($answer['correct']) {
                    $answers_correct++;
                  }
                }
              }
              if (!$answers_correct || !$answers) {
                $path = 'attach][tabs][content][task][content][' . $task_key . '][auto_test][question][' . $question_key;
                form_set_error($path,
                  t('The question must have at least one correct answer. (task @t, question @q)',
                    array(
                      '@t' => $task_key + 1,
                      '@q' => $question_key + 1,
                    )));
              }
              if (!$question['weight']) {
                $path = 'attach][tabs][content][task][content][' . $task_key . '][auto_test][question][' . $question_key;
                form_set_error($path,
                  t('Weight question must be greater than zero. (task @t, question @q)',
                    array(
                      '@t' => $task_key + 1,
                      '@q' => $question_key + 1,
                    )));
              }
              elseif (!is_numeric($question['weight']) || intval($question['weight']) != $question['weight'] || $question['weight'] <= 0) {
                $path = 'attach][tabs][content][task][content][' . $task_key . '][auto_test][question][' . $question_key;
                form_set_error($path,
                  t('Weight must be a positive integer. (task @t, question @q)',
                    array(
                      '@t' => $task_key + 1,
                      '@q' => $question_key + 1,
                    )));
              }
              else {
                $total_weight += $question['weight'];
              }
              $questions++;
            }
          }
        }

        if ($task['auto_test']['title']) {
          $approval_threshold = (int) $task['auto_test']['approval_threshold'];
          if (!$approval_threshold) {
            $path = 'attach][tabs][content][task][content][' . $task_key . '][auto_test][approval_threshold';
            form_set_error($path, t('!name field is required. (task @t)', array(
              '!name' => t('Approval threshold'),
              '@t'    => $task_key + 1,
            )));
          }
          elseif (!is_numeric($approval_threshold) || intval($approval_threshold) != $approval_threshold || $approval_threshold <= 0) {
            $path = 'attach][tabs][content][task][content][' . $task_key . '][auto_test][approval_threshold';
            form_set_error($path,
              t('Approval threshold must be a positive integer. (task @t)',
                array(
                  '@t' => $task_key + 1,
                )));
          }
          elseif ($approval_threshold > $total_weight) {
            $path = 'attach][tabs][content][task][content][' . $task_key . '][auto_test][approval_threshold';
            form_set_error($path,
              t('The total weight of questions must be greater than or equal to the threshold endorsement. (task @t)',
                array(
                  '@t' => $task_key + 1,
                )));
          }

          $time_completion = (int) $task['auto_test']['time_completion'];
          if (!$time_completion) {
            $path = 'attach][tabs][content][task][content][' . $task_key . '][auto_test][time_completion';
            form_set_error($path, t('!name field is required. (task @t)', array(
              '!name' => t('Time for completion'),
              '@t'    => $task_key + 1,
            )));
          }
          elseif (!is_numeric($time_completion) || intval($time_completion) != $time_completion || $time_completion < 1 || $time_completion > 100) {
            $path = 'attach][tabs][content][task][content][' . $task_key . '][auto_test][time_completion';
            form_set_error($path,
              t('Time for completion must be from 1 to 100 minutes. (task @t)',
                array(
                  '@t' => $task_key + 1,
                )));
          }
        }
        if ($questions && $answers) {
          $tasks_filled++;
        }
        break;

      case DITOOLSI_TRAINING_TASK_TYPE_TEXT_TASK:
        if ($task['text_task']['description']['value']) {
          $tasks_filled++;
        }
        if ($task['text_task']['redaction']
          && !$task['text_task']['first_redaction']['value']
        ) {
          $path = 'attach][tabs][content][task][content][' . $task_key . '][text_task][first_redaction';
          form_set_error($path, t('The task must have first redaction'));
        }
        break;
    }
  }

  if (!$tasks_filled) {
    form_set_error('', t('The lesson must have at least one job.'));
  }
}

/**
 * [ditoolsi_training_add_lesson_form_submit description]
 * @param  [type] $form        [description]
 * @param  [type] &$form_state [description]
 * @return [type]              [description]
 */
function ditoolsi_training_add_lesson_form_submit($form, &$form_state) {
  global $user;

  $values       = $form_state['values'];
  $info         = $values['info'];
  $attach       = $values['attach']['tabs']['content'];
  $video        = $attach['video']['content'];
  $audio        = $attach['audio']['content'];
  $presentation = $attach['presentation']['content'];
  $task         = $attach['task']['content'];
  $course       = $form_state['build_info']['args'][0];

  // Saving lesson.
  if (isset($form_state['build_info']['args'][1])) {
    $lesson = $form_state['build_info']['args'][1];
  }
  else {
    $lesson = entity_create('node', array(
      'type' => 'lesson',
      'uid'  => $user->uid,
    ));
  }

  $task_remove = array();

  if (!empty($form_state['storage']['task_to_remove'])) {
    $task_remove = $form_state['storage']['task_to_remove'];
  }

  $l_wrapper                            = entity_metadata_wrapper('node',
    $lesson);
  $l_wrapper->field_lesson_number       = $info['number'];
  $l_wrapper->title                     = $info['title'];
  $l_wrapper->field_lesson_link_post    = $info['link_post'];
  $l_wrapper->field_lesson_message_post = $info['message_post'];
  $l_wrapper->body                      = $info['body'];
  $l_wrapper->field_lesson_check_time   = (int) $info['check_time'];
  $l_wrapper->field_lesson_pass_time    = (int) $info['pass_time'];
  $l_wrapper->field_lesson_final        = (bool) $info['is_final'];
  $l_wrapper->field_lesson_course       = $course->nid;
  $l_wrapper->field_lesson_check_type   = $info['autocheck']['switcher'];
  $l_wrapper->field_low_score           = $info['autocheck']['low-score'];
  $l_wrapper->field_high_score          = $info['autocheck']['high-score'];
  $l_wrapper->field_autocomment         = $info['autocheck']['autocomments'];

  // Saving video.
  foreach ($video as $item) {
    if (!empty($item['item_id'])) {
      $fc_video = field_collection_item_load($item['item_id']);
      if (!$fc_video->hostEntity()) {
        $fc_video->updateHostEntity($lesson);
      }
    }
    else {
      $fc_video = entity_create('field_collection_item', array(
        'field_name' => 'field_lesson_video',
      ));
      $fc_video->setHostEntity('node', $lesson);
    }

    $v_wrapper = entity_metadata_wrapper('field_collection_item', $fc_video);
    $v_wrapper->field_lesson_video_title->set($item['title']);

    if (($oldfile = $v_wrapper->field_lesson_video_file->value()) && empty($item['file'])) {
      $oldfile = (object) $oldfile;
      file_delete($oldfile);
      file_usage_delete($oldfile, 'field_collection', $v_wrapper->getBundle(),
        $item['item_id']);
      $v_wrapper->field_lesson_video_file->set(NULL);
      $v_wrapper->field_lesson_video_url->set(NULL);
      $v_wrapper->field_embedded_code->set(NULL);
      $v_wrapper->save(TRUE);
    }
    elseif (empty($item['url']) && empty($item['file']) && empty($item['embedded_code'])) {
      $v_wrapper->field_lesson_video_file->set(NULL);
      $v_wrapper->field_lesson_video_url->set(NULL);
      $v_wrapper->field_embedded_code->set(NULL);
      $v_wrapper->field_lesson_video_title->set('');
      $v_wrapper->save(TRUE);
      continue;
    }

    $type = NULL;

    if (!empty($item['file'])) {
      $v_wrapper->field_lesson_video_type = DITOOLSI_TRAINING_VIDEO_TYPE_LOCAL;
      $file                               = file_load($item['file']);

      $file->status      = FILE_STATUS_PERMANENT;
      $file->display     = 1;
      $file->description = '';
      file_save($file);

      if ($oldfile = $v_wrapper->field_lesson_video_file->value()) {
        $oldfile = (object) $oldfile;
        file_delete($oldfile);
        file_usage_delete($oldfile, 'field_collection', $v_wrapper->getBundle(),
          $item['item_id']);
      }

      $v_wrapper->field_lesson_video_file = (array) $file;
      $v_wrapper->save(TRUE);
      file_usage_add($file, 'field_collection', $v_wrapper->getBundle(),
        $v_wrapper->getIdentifier());
    }
    elseif (!empty($item['url'])) {
      $type                               = DITOOLSI_TRAINING_VIDEO_TYPE_YOUTUBE;
      $v_wrapper->field_lesson_video_type = $type;
      $v_wrapper->field_lesson_video_url  = $item['url'];

      if ($file = $v_wrapper->field_lesson_video_file->value()) {
        $file = (object) $file;
      }
      elseif (!empty($item['file'])) {
        $file = file_load($item['file']);
      }

      if ($file) {
        file_delete($file);
        file_usage_delete($file, 'field_collection', $v_wrapper->getBundle(),
          $item['item_id']);
      }

      $v_wrapper->save(TRUE);
    }
    elseif (!empty($item['embedded_code'])) {
      $type                               = DITOOLSI_TRAINING_VIDEO_TYPE_CODE;
      $v_wrapper->field_lesson_video_type = $type;
      $v_wrapper->field_embedded_code     = $item['embedded_code'];
      $stop                               = 'Stop';
      if ($file = $v_wrapper->field_lesson_video_file->value()) {
        $file = (object) $file;
      }
      elseif (!empty($item['file'])) {
        $file = file_load($item['file']);
      }

      if ($file) {
        file_delete($file);
        file_usage_delete($file, 'field_collection', $v_wrapper->getBundle(),
          $item['item_id']);
      }

      $v_wrapper->save(TRUE);
    }
  }

  // Saving audio.
  foreach ($audio as $item) {
    if (!empty($item['item_id'])) {
      $fc_audio = field_collection_item_load($item['item_id']);

      if (!$fc_audio->hostEntity()) {
        $fc_audio->updateHostEntity($lesson);
      }
    }
    else {
      $fc_audio = entity_create('field_collection_item', array(
        'field_name' => 'field_lesson_audio',
      ));
      $fc_audio->setHostEntity('node', $lesson);
    }

    $v_wrapper = entity_metadata_wrapper('field_collection_item', $fc_audio);
    $v_wrapper->field_lesson_audio_title->set($item['title']);

    if (($oldfile = $v_wrapper->field_lesson_audio_file->value()) && empty($item['file'])) {
      $oldfile = (object) $oldfile;
      file_delete($oldfile);
      file_usage_delete($oldfile, 'field_collection', $v_wrapper->getBundle(),
        $item['item_id']);
      $v_wrapper->field_lesson_audio_file->set(NULL);
      $v_wrapper->field_lesson_audio_title->set('');
      $v_wrapper->save(TRUE);
    }
    elseif (empty($item['file'])) {
      $v_wrapper->field_lesson_audio_file->set(NULL);
      $v_wrapper->field_lesson_audio_title->set('');
      $v_wrapper->save(TRUE);
      continue;
    }

    if (!empty($item['file'])) {
      $file              = file_load($item['file']);
      $file->status      = FILE_STATUS_PERMANENT;
      $file->display     = 1;
      $file->description = '';
      file_save($file);

      if ($oldfile = $v_wrapper->field_lesson_audio_file->value()) {
        $oldfile = (object) $oldfile;
        file_delete($oldfile);
        file_usage_delete($oldfile, 'field_collection', $v_wrapper->getBundle(),
          $item['item_id']);
      }

      $v_wrapper->field_lesson_audio_file = (array) $file;
      $v_wrapper->save(TRUE);
      file_usage_add($file, 'field_collection', $v_wrapper->getBundle(),
        $v_wrapper->getIdentifier());
    }
  }

  // Saving presentations.
  foreach ($presentation as $item) {
    if (!empty($item['item_id'])) {
      $fc_presentation = field_collection_item_load($item['item_id']);
      if (!$fc_presentation->hostEntity()) {
        $fc_presentation->updateHostEntity($lesson);
      }
    }
    else {
      $fc_presentation = entity_create('field_collection_item', array(
        'field_name' => 'field_lesson_presentation',
      ));
      $fc_presentation->setHostEntity('node', $lesson);
    }

    $p_wrapper = entity_metadata_wrapper('field_collection_item',
      $fc_presentation);
    $p_wrapper->field_lesson_presentation_title->set($item['title']);

    if ($oldfile = $p_wrapper->field_lesson_presentation_file->value()) {
      $oldfile = (object) $oldfile;
      file_delete($oldfile);
      file_usage_delete($oldfile, 'field_collection', $p_wrapper->getBundle(),
        $item['item_id']);
    }

    if (empty($item['file'])) {
      $p_wrapper->field_lesson_presentation_title->set('');
      $p_wrapper->save(TRUE);
      continue;
    }

    $file              = file_load($item['file']);
    $file->status      = FILE_STATUS_PERMANENT;
    $file->display     = 1;
    $file->description = '';
    file_save($file);

    $p_wrapper->field_lesson_presentation_file = (array) $file;
    $p_wrapper->save(TRUE);
    file_usage_add($file, 'field_collection', $p_wrapper->getBundle(),
      $p_wrapper->getIdentifier());
  }

  $l_wrapper->save();

  // Saving tasks.
  foreach ($task as $item) {
    switch ($item['task_type']) {
      case DITOOLSI_TRAINING_TASK_TYPE_TEXT_TASK:
        $data = $item['text_task'];

        if (empty($data['description']['value']) && empty($item['item_id'])) {
          continue;
        }
        elseif (empty($data['description']['value'])) {
          node_delete($item['item_id']);
          continue;
        }
        elseif (!empty($item['item_id'])) {
          $node = node_load($item['item_id']);
        }
        else {
          $node = entity_create('node', array(
            'type' => 'lesson_text_task',
            'uid'  => $user->uid,
          ));
        }

        $title                                      = substr($data['description']['value'],
          0, 120);
        $t_wrapper                                  = entity_metadata_wrapper('node',
          $node);
        $t_wrapper->title                           = strip_tags($title);
        $t_wrapper->body                            = $data['description'];
        $t_wrapper->field_text_task_redaction       = $data['redaction'];
        $t_wrapper->field_text_task_first_redaction = $data['first_redaction'];

        if (!empty($data['file'])) {
          $file = file_load($data['file']);

          $file->status      = FILE_STATUS_PERMANENT;
          $file->display     = 1;
          $file->description = '';
          file_save($file);

          $t_wrapper->field_text_task_file = (array) $file;
        }
        else {
          $t_wrapper->field_text_task_file = NULL;
        }

        $t_wrapper->field_task_lesson = $l_wrapper->getIdentifier();
        $t_wrapper->save();
        if (!empty($data['file'])) {
          file_usage_add($file, 'node', 'lesson_text_task',
            $t_wrapper->getIdentifier());
        }
        break;

      case DITOOLSI_TRAINING_TASK_TYPE_AUTO_TEST:
        $data = $item['auto_test'];

        if ((empty($data['title']) || empty($data['questions'])) && empty($item['item_id'])) {
          continue;
        }
        elseif (empty($data['title']) || empty($data['questions'])) {
          node_delete($item['item_id']);
          continue;
        }
        elseif (!empty($item['item_id'])) {
          $node = node_load($item['item_id']);
        }
        else {
          $node = entity_create('node', array(
            'type' => 'lesson_auto_test',
            'uid'  => $user->uid,
          ));
        }

        $t_wrapper                                 = entity_metadata_wrapper('node',
          $node);
        $t_wrapper->title                          = $data['title'];
        $t_wrapper->field_autotest_freeze_time     = (int) $data['freeze_time'];
        $t_wrapper->field_autotest_threshold       = (int) $data['approval_threshold'];
        $t_wrapper->field_autotest_time_completion = (int) $data['time_completion'];
        $t_wrapper->field_task_lesson              = $l_wrapper->getIdentifier();

        $questions = array();

        foreach ($data['questions'] as $question) {
          if (empty($question['answers']) && empty($question['item_id'])) {
            continue;
          }
          elseif (empty($question['answers'])) {
            $fci = field_collection_item_load($question['item_id']);
            if ($fci) {
              entity_delete('field_collection_item', $question['item_id']);
            }
            continue;
          }
          elseif (!empty($question['item_id'])) {
            $fc_question = field_collection_item_load($question['item_id']);

            if (!$fc_question) {
              continue;
            }
            elseif (!$fc_question->hostEntity()) {
              $fc_question->updateHostEntity($node);
            }
          }
          else {
            $fc_question = entity_create('field_collection_item', array(
              'field_name' => 'field_autotest_questions',
            ));
            $fc_question->setHostEntity('node', $node);
          }

          $q_wrapper                                 = entity_metadata_wrapper('field_collection_item',
            $fc_question);
          $q_wrapper->field_autotest_question        = $question['title'];
          $q_wrapper->field_autotest_question_weight = (int) $question['weight'];

          $answers = array();

          foreach ($question['answers'] as $answer) {
            if (empty($answer['answer']) && empty($answer['item_id'])) {
              continue;
            }
            if (empty($answer['answer'])) {
              $fci = field_collection_item_load($answer['item_id']);
              if ($fci) {
                entity_delete('field_collection_item', $answer['item_id']);
              }
              continue;
            }
            elseif (!empty($answer['item_id'])) {
              $fc_answer = field_collection_item_load($answer['item_id']);
              if (!$fc_answer) {
                continue;
              }
              elseif ($fc_answer->hostEntity()) {
                $fc_answer->updateHostEntity($fc_question);
              }
            }
            else {
              $fc_answer = entity_create('field_collection_item', array(
                'field_name' => 'field_autotest_answers',
              ));
              $fc_answer->setHostEntity('field_collection_item', $fc_question);
            }

            $a_wrapper                                = entity_metadata_wrapper('field_collection_item',
              $fc_answer);
            $a_wrapper->field_question_answer         = $answer['answer'];
            $a_wrapper->field_question_answer_correct = (bool) $answer['correct'];
            $a_wrapper->save();
            $answers[] = $a_wrapper->getIdentifier();
          }

          $question = $q_wrapper->value();

          if (isset($question->field_autotest_answers[LANGUAGE_NONE])) {
            foreach ($question->field_autotest_answers[LANGUAGE_NONE] as $key => $ids) {
              if (!in_array($ids['value'], $answers)) {
                unset($question->field_autotest_answers[LANGUAGE_NONE][$key]);
              }
            }
          }

          $q_wrapper->save();
          $questions[] = $q_wrapper->getIdentifier();
        }

        $task = $t_wrapper->value();

        if (isset($task->field_autotest_questions[LANGUAGE_NONE])) {
          foreach ($task->field_autotest_questions[LANGUAGE_NONE] as $key => $ids) {
            if (!in_array($ids['value'], $questions)) {
              unset($task->field_autotest_questions[LANGUAGE_NONE][$key]);
            }
          }
        }

        $t_wrapper->save();
        break;
    }
  }

  foreach ($task_remove as $item) {
    node_delete($item);
  }

  if ($l_wrapper->getIdentifier()) {
    $op = 'update';
  }
  else {
    $op = 'create';
  }

  $l_wrapper->save();
  $nid = $l_wrapper->getIdentifier();

  if ($nid) {
    if ($op == 'update') {
      drupal_set_message(t('Lesson has been updated'));
    }
    else {
      drupal_set_message(t('Lesson has been created'));
    }

    $form_state['nid']      = $nid;
    $form_state['redirect'] = 'course/' . $course->nid . '/' . $nid . '/edit';
  }
  else {
    drupal_set_message(t('An error occurred'));
  }
}

function ditoolsi_training_redirect_after_delete_lesson($form, &$form_state) {
  $course                 = $form_state['build_info']['args'][0];
  $lesson                 = $form_state['build_info']['args'][1];
  $form_state['redirect'] = 'course/' . $course->nid . '/' . $lesson->nid . '/delete';
}

/**
 * Form for delete lesson
 */
function ditoolsi_training_delete_lesson_form(
  $form,
  &$form_state,
  $course,
  $lesson
) {
  $course        = $form_state['build_info']['args'][0];
  $lesson        = $form_state['build_info']['args'][1];
  $form['#node'] = $lesson;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['lesson'] = array('#type' => 'value', '#value' => $lesson->nid);
  $form['course'] = array('#type' => 'value', '#value' => $course->nid);

  return confirm_form($form,
    t('Are you sure you want to delete lesson %title?',
      array('%title' => $lesson->title)),
    'course/' . $course->nid,
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Form submit function for delete lesson
 */
function ditoolsi_training_delete_lesson_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $lesson = node_load($form_state['values']['lesson']);
    $course = $form_state['values']['course'];

    if ($lesson->type == 'lesson') {
      $query = db_select('field_data_field_task_lesson', 'tl');
      $query->fields('tl', array('entity_id'));
      $query->condition('tl.field_task_lesson_target_id', $lesson->nid);
      $tasks = $query->execute()->fetchAll();

      $delete_nids = array();
      if (!empty($tasks)) {
        foreach ($tasks as $task) {
          $delete_nids[] = $task->entity_id;
        }
      }

      $delete_nids[] = $lesson->nid;
      node_delete_multiple($delete_nids);
      watchdog('content', '@type: deleted %title.', array(
        '@type'  => $lesson->type,
        '%title' => $lesson->title
      ));
      drupal_set_message(t('Lesson was deleted'), 'status', FALSE);
      $form_state['redirect'] = "course/" . $course;
    }
  }
}

/**
 * Form for join pupil to course.
 */
function ditoolsi_training_add_curator_form($form, &$form_state) {
  global $user;

  $account = ditoolsi_profile_account($user);

  $query  = ditoolsi_training_get_cources_query();
  $result = $query->execute()->fetchAllKeyed(0, 1);

  $nids = array_keys($result);

  $nodes   = node_load_multiple($nids);
  $options = array();

  foreach ($nodes as $node) {
    $options[$node->nid] = $node->title;
  }

  $form['#tree']       = TRUE;
  $form['#id']         = 'add-curator-form';
  $form['#attributes'] = array(
    'class' => array(
      'process-ditools-labels',
    ),
  );

  $form['manual'] = array(
    '#type'       => 'fieldset',
    '#id'         => 'add-curator-fieldset',
    '#attributes' => array(
      'class' => array(
        'process-ditools-labels',
      ),
    ),
  );

  $form['manual']['mail'] = array(
    '#type'     => 'textfield',
    '#title'    => 'E-mail',
    '#required' => TRUE,
  );

  $form['manual']['course'] = array(
    '#type'       => 'select',
    '#options'    => $options,
    '#attributes' => array(
      'class' => array(
        'ks-selecter-attach',
      ),
    ),
    '#ajax'       => array(
      'callback' => 'ditoolsi_training_add_curator_ajax',
      'wrapper'  => 'add-curator-form',
    ),
  );

  $form['manual']['submit'] = array(
    '#type'       => 'submit',
    '#value'      => t('Add curator'),
    '#submit'     => array('ditoolsi_training_add_curator_submit_ajax'),
    '#ajax'       => array(
      'callback' => 'ditoolsi_training_add_curator_ajax',
      'wrapper'  => 'add-curator-form',
      'event'    => 'click',
    ),
    '#attributes' => array(
      'class' => array(
        'dit-submit-blue',
      ),
    ),
  );

  $form['list'] = array(
    '#type'  => 'item',
    '#id'    => 'list-curators',
    '#title' => t('List curators'),
  );

  if (isset($form_state['values']['manual']['course'])) {
    $course_id = $form_state['values']['manual']['course'];
  }
  elseif (!empty($options)) {
    $options   = array_keys($options);
    $course_id = reset($options);
  }
  else {
    $course_id = FALSE;
  }

  $query = db_select('ditoolsi_training_curators_list', 'cl');
  $query->join('users', 'u', 'u.uid = cl.user_id');
  $query->addField('u', 'uid');
  $query
    ->condition('cl.course_id', $course_id)
    ->condition('u.uid', 0, '<>');
  $uids = $query->execute()->fetchCol();

  if ($uids) {
    $accounts = user_load_multiple($uids);

    foreach ($accounts as $account) {
      $name                        = ditoolsi_profile_name($account);
      $form['list'][$account->uid] = array(
        '#type' => 'container',
      );

      $form['list'][$account->uid]['name'] = array(
        '#type'   => 'item',
        '#markup' => l($name, 'user/' . $account->uid),
      );

      $form['list'][$account->uid]['op'] = array(
        '#type'                    => 'submit',
        '#value'                   => t('Delete'),
        '#prefix'                  => '<div class="op">',
        '#suffix'                  => '</div>',
        '#name'                    => 'delete_curator[' . $account->uid . ']',
        '#submit'                  => array('ditoolsi_training_delete_curator_submit_ajax'),
        '#ajax'                    => array(
          'callback' => 'ditoolsi_training_add_curator_ajax',
          'wrapper'  => 'add-curator-form',
          'event'    => 'click',
        ),
        '#limit_validation_errors' => array(
          array('manual', 'course'),
        ),
      );
    }
  }
  else {
    $form['list']['#markup'] = '<div class="empty-text">' . t('On this course there are no curators') . '</div>';
  }


  $form['#attached'] = array(
    'css' => array(
      drupal_get_path('theme', 'bootstrap_subtheme') . '/css/add-curator.css',
    ),
  );

  $form['#attributes'] = array(
    'class' => array(
      'clearfix',
    ),
  );

  return $form;
}

function ditoolsi_training_delete_curator_submit_ajax($form, &$form_state) {
  $user_id   = $form_state['triggering_element']['#parents'][1];
  $course_id = $form_state['values']['manual']['course'];

  db_delete('ditoolsi_training_curators_list')
    ->condition('course_id', $course_id)
    ->condition('user_id', $user_id)
    ->execute();

  $is_curator = db_select('ditoolsi_training_curators_list', 'cl')
    ->condition('cl.user_id', $user_id)
    ->countQuery()
    ->execute()
    ->fetchField();

  if (!$is_curator) {
    $account = user_load($user_id);
    if ($account && isset($account->roles[DITOOLSI_CURATOR_RID])) {
      $user_roles = $account->roles;
      unset($user_roles[DITOOLSI_CURATOR_RID]);
      user_save($account, array('roles' => $user_roles));
    }
  }

  $form_state['rebuild'] = TRUE;
}

function ditoolsi_training_add_curator_ajax($form, $form_state) {
  return $form;
}

function ditoolsi_training_add_curator_submit_ajax($form, &$form_state) {
  $mail = $form_state['values']['manual']['mail'];
  if (valid_email_address($mail)) {
    $account = user_load_by_mail($mail);
    if (!empty($account)) {
      $course_id = $form_state['values']['manual']['course'];

      db_merge('ditoolsi_training_curators_list')
        ->fields(array(
          'course_id' => $course_id,
          'user_id'   => $account->uid,
          'created'   => REQUEST_TIME,
        ))
        ->key(array(
          'course_id' => $course_id,
          'user_id'   => $account->uid,
        ))
        ->execute();

      $roles = user_roles();

      if (isset($roles[DITOOLSI_CURATOR_RID])) {
        $user_roles                       = $account->roles;
        $user_roles[DITOOLSI_CURATOR_RID] = $roles[DITOOLSI_CURATOR_RID];
        user_save($account, array('roles' => $user_roles));
      }
      $course_node  = node_load($course_id);
      $mail_subject = 'Вас добавили в качестве куратора на курс "' . $course_node->title . '"';
      $mail_body    = 'Вас добавили в качестве куратора на курс "' . $course_node->title . '"<br>
         Ссылка для входа на сайт: <a href="http://rewrite.up-lvl.ru/user">http://rewrite.up-lvl.ru/user</a><br>
         Для входа используйте следующие данные:         
         Логин: <b>' . $account->mail;
      /*mgc_send_mail($account->mail, $mail[0]['subject'], $mail[0]['body']);
  */
      global $language;
      drupal_mail('system', __FUNCTION__, $mail, $language, array(
        'context' => array(
          'subject' => $mail_subject,
          'message' => $mail_body,
        ),
      ));
    }
    else {
      $roles                       = array();
      $roles[DITOOLSI_CURATOR_RID] = DITOOLSI_CURATOR_RID;

      $fields  = array(
        'name'   => $mail,
        'mail'   => $mail,
        'pass'   => DITOOLSI_DEFAULT_PASS_USER,
        'status' => 1,
        'access' => time(),
        'roles'  => $roles,
        'init'   => $form_state['values']['manual']['course'],
      );
      $account = user_save('', $fields);

      $account = user_load_by_mail($mail); // Loading account

      $course_id = $form_state['values']['manual']['course'];

      $query        = db_insert('ditoolsi_training_curators_list')
        ->fields(array(
          'course_id' => $course_id,
          'user_id'   => $account->uid,
          'created'   => REQUEST_TIME,
        ))
        ->execute();
      $course_node  = node_load($course_id);
      $mail_subject = 'Вас добавили в качестве куратора на курс "' . $course_node->title . '"';
      $mail_body    = 'Вас добавили в качестве куратора на курс "' . $course_node->title . '"<br>
         Ссылка для входа на сайт: <a href="http://rewrite.up-lvl.ru/user">http://rewrite.up-lvl.ru/user</a><br>
         Для входа используйте следующие данные:         
         Логин: <b>' . $fields['name'] . '</b> пароль:<b>' . $fields['pass'] . '</b>' .
        '<br><br>';
//        '<br><br>* <em>Обязательно поменяйте стандартный пароль на свой!</em><br>';
      /*mgc_send_mail($account->mail, $mail[0]['subject'], $mail[0]['body']);
  */
      global $language;
      drupal_mail('system', __FUNCTION__, $mail, $language, array(
        'context' => array(
          'subject' => $mail_subject,
          'message' => $mail_body,
        ),
      ));
    }


    drupal_set_message(t('User joined to course'));

    if (isset($form_state['values']['manual']['mail'])) {
      unset($form_state['values']['manual']['mail']);
    }
    if (isset($form_state['input']['manual']['mail'])) {
      unset($form_state['input']['manual']['mail']);
    }
  }
  else {
    drupal_set_message(t('Wrong e-mail format @email',
      array('@email' => $form_state['values']['manual']['mail'])), 'error');
  }

  $form_state['rebuild'] = TRUE;
}

function ditoolsi_training_save_task_form(
  $form,
  &$form_state,
  $lesson,
  $task,
  $progress = NULL
) {
  global $user;

  $form['#id'] = 'save-task-form-' . $lesson->nid . '-' . $task->nid;

  if (empty($progress->uid)) {
    $form['log'] = array(
      '#type' => 'container',
    );

    $query = new EntityFieldQuery();
    $query
      ->entityCondition('entity_type', 'node')
      ->entityCondition('bundle', 'text_task_progress')
      ->propertyCondition('uid', $user->uid)
      ->fieldCondition('field_progress_task', 'target_id', $task->nid);

    $result = $query->execute();

    if (isset($result['node'])) {
      $nid           = key($result['node']);
      $progress_task = node_load($nid);
      $pt_wrapper    = entity_metadata_wrapper('node', $progress_task);

      foreach ($pt_wrapper->field_progress_comments->value() as $key => $comment) {
        $form['log'][$key] = array(
          '#type'  => 'item',
          '#title' => t('Answer of curator'),
        );

        $form['log'][$key]['comment'] = array(
          '#markup' => '<div class="teacher-comment">' . check_markup($comment['value'],
              $comment['format']) . '</div>',
        );
      }
    }
  }

  $form['answer'] = array(
    '#type'     => 'text_format',
    '#format'   => 'filtered_html',
    '#title'    => t('Your answer'),
    '#weight'   => 1,
    '#required' => TRUE,
  );

  if (!empty($progress->uid)) {
    $form['answer']['#title'] = t("Answer pupil's");
    $uid                      = $progress->uid;

    $form['pupil_files'] = array(
      '#type'   => 'item',
      '#title'  => t('Pupil files to the task'),
      '#weight' => 1.9,
    );

    $form['submit'] = array(
      '#type'       => 'submit',
      '#value'      => t('Accept the task'),
      '#submit'     => array('ditoolsi_training_accept_task_submit_ajax'),
      '#attributes' => array(
        'class' => array(
          'dit-submit-blue',
        ),
      ),
      '#ajax'       => array(
        'callback' => 'ditoolsi_training_accept_task_ajax',
        'wrapper'  => 'save-task-form-' . $lesson->nid . '-' . $task->nid,
        'event'    => 'click',
      ),
      '#weight'     => 3,
    );

    $form['to_fix'] = array(
      '#markup' => l(t('To fix'), '', array(
        'attributes' => array(
          'class' => array(
            'toggle-form',
            'to-fix',
          ),
        ),
      )),
      '#weight' => 3.1,
    );
  }
  else {
    $uid = $user->uid;

    $form['submit'] = array(
      '#type'       => 'submit',
      '#value'      => t('Save'),
      '#submit'     => array('ditoolsi_training_save_task_submit_ajax'),
      '#attributes' => array(
        'class' => array(
          'dit-submit-blue',
        ),
      ),
      '#ajax'       => array(
        'callback' => 'ditoolsi_training_send_review_ajax',
        'wrapper'  => 'save-task-form-' . $lesson->nid . '-' . $task->nid,
        'event'    => 'click',
      ),
      '#weight'     => 3,
    );

  }
  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'text_task_progress')
    ->propertyCondition('uid', $uid)
    ->fieldCondition('field_progress_task', 'target_id', $task->nid);

  $result = $query->execute();

  if (isset($result['node'])) {
    $nid                              = key($result['node']);
    $progress_task                    = node_load($nid);
    $pt_wrapper                       = entity_metadata_wrapper('node',
      $progress_task);
    $body                             = $pt_wrapper->body->value();
    $form['answer']['#default_value'] = $body['value'];

    if (!empty($progress->uid)) {
      $form['pupil_files'] = array(
        '#tree'   => TRUE,
        '#prefix' => '<div class="files-pupil">',
        '#suffix' => '</div>',
      );

      $form['pupil_files']['title'] = array(
        '#type'  => 'item',
        '#title' => t('Files'),
      );

      foreach ($pt_wrapper->field_progress_files->value() as $file) {
        $url                               = file_create_url($file['uri']);
        $form['pupil_files'][$file['fid']] = array(
          '#markup' => '<div class="file-item">' . l($file['filename'], $url,
              array(
                'attributes' => array(
                  'target' => '_blank',
                ),
              )) . '</div>',
        );
      }

      $form['pupils_link'] = array(
        '#tree'   => TRUE,
        '#prefix' => '<div class="links-pupil">',
        '#suffix' => '</div>',
      );

      $form['pupils_link']['title'] = array(
        '#type'  => 'item',
        '#title' => t('Links'),
      );

      // foreach ($pt_wrapper->field_progress_links->value() as $link) {

      //   $form['pupils_link'][$link] = array(
      //      '#prefix' => '<div class="pupils-link">',
      //      '#markup' => l($link, $link, array(
      //         'external' => FALSE,
      //         'attributes' => array(
      //           'target' => '_blank',
      //           "rel"=>"nofollow",
      //           ),
      //       )),
      //      '#suffix' => '</div>',
      //     );
      // }
    }
  }
  else {
    $progress_task = entity_create('node', array(
      'type' => 'text_task_progress',
      'uid'  => $user->uid,
    ));
  }

  if (empty($progress->uid)) {
    $form_tmp = $form;
    field_attach_form('node', $progress_task, $form_tmp, $form_state);

    $form['field_progress_files']            = $form_tmp['field_progress_files'];
    $form['field_progress_files']['#weight'] = 2;

    // $form_tmp = $form;
    // field_attach_form('node', $progress_task, $form_tmp, $form_state);

    // $form['field_progress_links'] = $form_tmp['field_progress_links'];
    // $form['field_progress_links']['#weight'] = 2;
  }

  $form['#after_build'][] = 'ditoolsi_training_save_task_form_after_build';

  form_load_include($form_state, 'inc', 'ditoolsi_training', 'includes/form');

  return $form;
}

function ditoolsi_training_accept_task_ajax($form, $form_state) {
  $lesson              = $form_state['build_info']['args'][0];
  $task                = $form_state['build_info']['args'][1];
  $current_task_number = NULL;
  $key                 = 1;

  foreach ($lesson->tasks as $nid => $task_item) {
    if ($task->nid == $task_item->nid) {
      $current_task_number = ++$key;
      break;
    }

    $key++;
  }

  $commands   = array();
  $commands[] = array(
    'command' => 'ditoolsiTrainingSwitchTask',
    'task'    => $current_task_number,
    'class'   => 'status-finished',
  );

  return array(
    '#type'     => 'ajax',
    '#commands' => $commands,
  );
}

function ditoolsi_training_accept_task_submit_ajax($form, &$form_state) {
  $lesson   = $form_state['build_info']['args'][0];
  $task     = $form_state['build_info']['args'][1];
  $progress = $form_state['build_info']['args'][2];

  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'text_task_progress')
    ->propertyCondition('uid', $progress->uid)
    ->fieldCondition('field_progress_task', 'target_id', $task->nid);

  $result = $query->execute();

  $nid           = key($result['node']);
  $progress_task = node_load($nid);
  $pt_wrapper    = entity_metadata_wrapper('node', $progress_task);
  $pl_wrapper    = entity_metadata_wrapper('node', $progress);

  $items = $pl_wrapper->field_progress_tasks->value();

  foreach ($items as $item) {
    $fc_wrapper = entity_metadata_wrapper('field_collection_item', $item);
    if ($fc_wrapper->field_progress_task_progress->raw() == $pt_wrapper->getIdentifier()) {
      $fc_wrapper->field_progress_status->set(DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED);
      $fc_wrapper->save();
      break;
    }
  }

  if (!empty($form_state['values']['to_fix'])) {
    $pt_wrapper->field_progress_comments[NULL]->set($form_state['values']['to_fix']);
  }

  $pt_wrapper->save();
  $pl_wrapper->save();

  $form_state['allow_accept'] = ditoolsi_training_is_lesson_all_finished($lesson,
    $progress->uid);
  $form_state['rebuild']      = TRUE;
}

function ditoolsi_training_save_task_form_after_build($form, &$form_state) {
  $form['answer']['format']['#access'] = FALSE;

  return $form;
}

function ditoolsi_training_save_task_submit_ajax($form, &$form_state) {
  global $user;

  $lesson_id = $form_state['build_info']['args'][0]->nid;
  $task_id   = $form_state['build_info']['args'][1]->nid;

  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'text_task_progress')
    ->propertyCondition('uid', $user->uid)
    ->fieldCondition('field_progress_task', 'target_id', $task_id);

  $result = $query->execute();

  if (isset($result['node'])) {
    $nid           = key($result['node']);
    $progress_task = node_load($nid);
    $pt_wrapper    = entity_metadata_wrapper('node', $progress_task);
  }
  else {
    $progress_task                      = entity_create('node', array(
      'type'  => 'text_task_progress',
      'uid'   => $user->uid,
      'title' => 'Progress of task [nid:' . $task_id . '] for lesson [nid:' . $lesson_id . '] for user [uid:' . $user->uid . ']',
    ));
    $progress_task->field_progress_task = array(
      LANGUAGE_NONE => array(
        0 => array(
          'target_id' => $task_id,
        ),
      ),
    );
    $pt_wrapper                         = entity_metadata_wrapper('node',
      $progress_task);
    $pt_wrapper->save();
  }

  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'lesson_progress')
    ->propertyCondition('uid', $user->uid)
    ->fieldCondition('field_progress_lesson_id', 'target_id', $lesson_id);

  $result = $query->execute();

  if (isset($result['node'])) {
    $nid        = key($result['node']);
    $progress   = node_load($nid);
    $pl_wrapper = entity_metadata_wrapper('node', $progress);
  }
  else {
    $progress                           = entity_create('node', array(
      'type'  => 'lesson_progress',
      'uid'   => $user->uid,
      'title' => 'Progress of lesson [nid:' . $lesson_id . '] for user [uid:' . $user->uid . ']',
    ));
    $progress->field_progress_lesson_id = array(
      LANGUAGE_NONE => array(
        0 => array(
          'target_id' => $lesson_id,
        ),
      ),
    );
    $progress->field_progress_status    = array(
      LANGUAGE_NONE => array(
        0 => array(
          'value' => DITOOLSI_TRAINING_PROGRESS_STATUS_NEW,
        ),
      ),
    );
    $pl_wrapper                         = entity_metadata_wrapper('node',
      $progress);
  }
  $pl_wrapper->save();

  $items  = $pl_wrapper->field_progress_tasks->value();
  $exists = FALSE;

  foreach ($items as $item) {
    $fc_wrapper = entity_metadata_wrapper('field_collection_item', $item);
    if ($fc_wrapper->field_progress_task_progress->raw() == $pt_wrapper->getIdentifier()) {
      $exists = TRUE;
      $fc_wrapper->field_progress_status->set(DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW);
      $fc_wrapper->save();
      break;
    }
  }

  if (!$exists) {
    $fc_item = entity_create('field_collection_item', array(
      'field_name' => 'field_progress_tasks',
    ));
    $fc_item->setHostEntity('node', $progress);
    $fc_wrapper = entity_metadata_wrapper('field_collection_item', $fc_item);
    $fc_wrapper->field_progress_task_progress->set($pt_wrapper->getIdentifier());
    $fc_wrapper->field_progress_status->set(DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW);
    $fc_wrapper->save();
  }

  $pt_wrapper->body->set($form_state['values']['answer']);
  $pt_wrapper->field_progress_files->set(NULL);

  foreach ($form_state['values']['field_progress_files'][LANGUAGE_NONE] as $key => $value) {
    if (!$value['fid']) {
      continue;
    }
    $file            = (array) file_load($value['fid']);
    $file['status']  = FILE_STATUS_PERMANENT;
    $file['display'] = 1;
    $pt_wrapper->field_progress_files[NULL]->set($file);
    $file = (object) $file;
    file_save($file);
  }

  // $pt_wrapper->field_progress_links->set(NULL);
  // foreach ($form_state['values']['field_progress_links'][LANGUAGE_NONE] as $key => $value) {
  //   if (isset($value['value']) || !($value = trim($value['value']))) {
  //     continue;
  //   }

  //   $pt_wrapper->field_progress_links[NULL]->set($value);
  // }

  $pt_wrapper->save();
  $form_state['rebuild'] = TRUE;
}

function ditoolsi_training_send_review_ajax($form, $form_state) {
  global $user;

  if (form_get_errors()) {
    return $form;
  }

  $lesson             = $form_state['build_info']['args'][0];
  $course_id          = $lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'];
  $is_lesson_finished = ditoolsi_training_lesson_allowed_for_review($lesson,
    $user);

  if ($is_lesson_finished) {
    $text = t('You have completed all the tasks for the lesson, send a lesson to check');
    $text .= l(t('Send for review'),
      'course/' . $course_id . '/' . $lesson->nid . '/to-review', array(
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
  else {
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
    $progress         = ditoolsi_training_get_lesson_progress($lesson, $user);
    $allowed          = 0;

    foreach ($progress['tasks'] as $item) {
      if (in_array($item['status'], $allowed_statuses[$item['node']->type])) {
        $allowed++;
      }
    }
    $stop = 'Stop';

    $text = t('You have saved the completed task №@number to the lesson, you need to perform even @out out of @total jobs lesson',
      array(
        '@number' => $lesson->field_lesson_number[LANGUAGE_NONE][0]['value'],
        '@out'    => count($lesson->tasks) - $allowed,
        '@total'  => count($lesson->tasks),
      ));
    $text .= '<br />' . l('Перейти к следующему заданию', '', array(
        'attributes' => array(
          'class'    => array(
            'btn',
            'btn-default',
            'dit-submit',
          ),
          'data-idd' => $lesson->field_lesson_number[LANGUAGE_NONE][0]['value'],
        ),
      ));
  }
  if (isset($allowed)) {
    $data_count = $allowed;
  }
  else {
    $data_count = 0;
  }
  $commands   = array();
  $commands[] = ajax_command_remove('#task-saved-popup, #ditoolsi-overlay');
  $commands[] = ajax_command_replace('.task-progress-count-invisible',
    '<span class="task-progress-count-invisible" data-count="' . $data_count . '"></span>');
  $commands[] = ajax_command_append('body',
    '<div id="task-saved-popup">' . $text . '</div><div id="ditoolsi-overlay"></div>');

  return array(
    '#type'     => 'ajax',
    '#commands' => $commands,
  );
}

function ditoolsi_training_save_task_redaction_form(
  $form,
  &$form_state,
  $lesson,
  $task,
  $progress = NULL
) {
  global $user;

  if (!empty($progress)) {
    $account = user_load($progress->uid);
  }
  else {
    $account = $user;
  }

  $form['#id'] = 'save-task-redaction-form-' . $lesson->nid . '-' . $task->nid;
  //Get default value
  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'text_task_progress')
    ->propertyCondition('uid', $account->uid)
    ->fieldCondition('field_progress_task', 'target_id', $task->nid);

  $result = $query->execute();

  if (isset($result['node'])) {
    $nid           = key($result['node']);
    $progress_task = node_load($nid);
    $pt_wrapper    = entity_metadata_wrapper('node', $progress_task);

    if ($body_count = $pt_wrapper->field_progress_comments->count()) {
      $default_body = $pt_wrapper->field_progress_comments[$body_count - 1]->value();
    }
  }
  else {
    $default_body = $task->field_text_task_first_redaction[LANGUAGE_NONE][0];
  }
  $default_body_value = isset($default_body['value']) ? $default_body['value'] : '';
  $form['answer']     = array(
    '#type'          => 'text_format',
    '#default_value' => $default_body_value,
    '#format'        => 'filtered_html',
    '#title'         => t('Your answer'),
    '#weight'        => 1,
  );

  //get submit button
  if (!empty($progress)) {
    $form['submit'] = array(
      '#type'       => 'submit',
      '#value'      => t('Accept the task'),
      '#submit'     => array('ditoolsi_training_accept_task_redaction_submit'),
      '#attributes' => array(
        'class' => array(
          'dit-submit-blue',
        ),
      ),
      '#ajax'       => array(
        'callback' => 'ditoolsi_training_accept_task_redaction_submit_ajax',
        'wrapper'  => 'save-task-form-' . $lesson->nid . '-' . $task->nid,
        'event'    => 'click',
      ),
      '#weight'     => 3,
    );

    $form['fix'] = array(
      '#type'       => 'submit',
      '#value'      => t('To fix'),
      '#submit'     => array('ditoolsi_training_to_fix_task_redaction_submit'),
      '#attributes' => array(
        'class' => array(
          'dit-submit-blue',
        ),
      ),
      '#ajax'       => array(
        'callback' => 'ditoolsi_training_accept_task_redaction_submit_ajax',
        'wrapper'  => 'save-task-form-' . $lesson->nid . '-' . $task->nid,
        'event'    => 'click',
      ),
      '#weight'     => 4,
    );
  }
  else {
    $is_lesson_finished = ditoolsi_training_is_lesson_complete($lesson,
      $account);

    if (!$is_lesson_finished) {
      $form['submit'] = array(
        '#type'       => 'submit',
        '#value'      => t('Save'),
        '#submit'     => array('ditoolsi_training_save_task_redaction_pupil_submit'),
        '#attributes' => array(
          'class' => array(
            'dit-submit-blue',
          ),
        ),
        '#ajax'       => array(
          'callback' => 'ditoolsi_training_send_task_redaction_pupil_submit_ajax',
          'wrapper'  => 'save-task-form-' . $lesson->nid . '-' . $task->nid,
          'event'    => 'click',
        ),
        '#weight'     => 3,
      );
    }
  }

  $form['#after_build'][] = 'ditoolsi_training_save_task_form_after_build';

  form_load_include($form_state, 'inc', 'ditoolsi_training', 'includes/form');

  return $form;
}

function ditoolsi_training_save_task_redaction_pupil_submit(
  $form,
  &$form_state
) {
  global $user;
  $lesson_id = $form_state['build_info']['args'][0]->nid;
  $task      = $form_state['build_info']['args'][1];
  $task_id   = $task->nid;

  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'text_task_progress')
    ->propertyCondition('uid', $user->uid)
    ->fieldCondition('field_progress_task', 'target_id', $task_id);

  $result = $query->execute();

  if (isset($result['node'])) {
    $nid           = key($result['node']);
    $progress_task = node_load($nid);
    $pt_wrapper    = entity_metadata_wrapper('node', $progress_task);
  }
  else {
    $progress_task                      = entity_create('node', array(
      'type'  => 'text_task_progress',
      'uid'   => $user->uid,
      'title' => 'Progress of task [nid:' . $task_id . '] for lesson [nid:' . $lesson_id . '] for user [uid:' . $user->uid . ']',
    ));
    $progress_task->field_progress_task = array(
      LANGUAGE_NONE => array(
        0 => array(
          'target_id' => $task_id,
        ),
      ),
    );
    $pt_wrapper                         = entity_metadata_wrapper('node',
      $progress_task);
    $pt_wrapper->save();
  }

  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'lesson_progress')
    ->propertyCondition('uid', $user->uid)
    ->fieldCondition('field_progress_lesson_id', 'target_id', $lesson_id);

  $result = $query->execute();

  if (isset($result['node'])) {
    $nid        = key($result['node']);
    $progress   = node_load($nid);
    $pl_wrapper = entity_metadata_wrapper('node', $progress);
  }
  else {
    $progress                           = entity_create('node', array(
      'type'  => 'lesson_progress',
      'uid'   => $user->uid,
      'title' => 'Progress of lesson [nid:' . $lesson_id . '] for user [uid:' . $user->uid . ']',
    ));
    $progress->field_progress_lesson_id = array(
      LANGUAGE_NONE => array(
        0 => array(
          'target_id' => $lesson_id,
        ),
      ),
    );
    $progress->field_progress_status    = array(
      LANGUAGE_NONE => array(
        0 => array(
          'value' => DITOOLSI_TRAINING_PROGRESS_STATUS_NEW,
        ),
      ),
    );
    $pl_wrapper                         = entity_metadata_wrapper('node',
      $progress);
  }

  $pl_wrapper->save();

  $items  = $pl_wrapper->field_progress_tasks->value();
  $exists = FALSE;

  $progress_status = ditoolsi_training_get_task_progress($task, $user);
  if ($progress_status == DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW) {
    $redaction                                       = count($pt_wrapper->field_progress_comments) - 1;
    $pt_wrapper->field_progress_comments[$redaction] = $form_state['values']['answer'];
  }
  else {
    $pt_wrapper->field_progress_comments[] = $form_state['values']['answer'];
  }
  $pt_wrapper->field_progress_files->set(NULL);
  // $pt_wrapper->field_progress_links->set(NULL);

  $pt_wrapper->save();

  foreach ($items as $item) {
    $fc_wrapper = entity_metadata_wrapper('field_collection_item', $item);
    if ($fc_wrapper->field_progress_task_progress->raw() == $pt_wrapper->getIdentifier()) {
      $exists = TRUE;
      $fc_wrapper->field_progress_status->set(DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW);
      $fc_wrapper->save();
      break;
    }
  }

  if (!$exists) {
    $fc_item = entity_create('field_collection_item', array(
      'field_name' => 'field_progress_tasks',
    ));
    $fc_item->setHostEntity('node', $progress);
    $fc_wrapper = entity_metadata_wrapper('field_collection_item', $fc_item);
    $fc_wrapper->field_progress_task_progress->set($pt_wrapper->getIdentifier());
    $fc_wrapper->field_progress_status->set(DITOOLSI_TRAINING_PROGRESS_STATUS_TO_REVIEW);
    $fc_wrapper->save();
  }

  $form_state['rebuild'] = TRUE;
}

function ditoolsi_training_send_task_redaction_pupil_submit_ajax(
  $form,
  $form_state
) {
  $commands = array();
  ctools_include('ajax');
  drupal_set_message(t('Your answer has been saved'));
  $lesson     = $form_state['build_info']['args'][0];
  $course_id  = $lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'];
  $commands[] = ctools_ajax_command_redirect("course/" . $course_id . '/' . $lesson->nid,
    0);

  return array(
    '#type'     => 'ajax',
    '#commands' => $commands,
  );
}

function ditoolsi_training_accept_task_redaction_submit($form, &$form_state) {
  $lesson   = $form_state['build_info']['args'][0];
  $task     = $form_state['build_info']['args'][1];
  $progress = $form_state['build_info']['args'][2];
  $account  = user_load($progress->uid);

  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'text_task_progress')
    ->propertyCondition('uid', $progress->uid)
    ->fieldCondition('field_progress_task', 'target_id', $task->nid);

  $result = $query->execute();

  $nid           = key($result['node']);
  $progress_task = node_load($nid);
  $pt_wrapper    = entity_metadata_wrapper('node', $progress_task);
  $pl_wrapper    = entity_metadata_wrapper('node', $progress);

  $items  = $pl_wrapper->field_progress_tasks->value();
  $exists = FALSE;

  $progress_status = ditoolsi_training_get_task_progress($task, $account);
  if ($progress_status == DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED
    || $progress_status == DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST
  ) {
    $redaction = count($pt_wrapper->field_progress_comments) - 1;
    $pt_wrapper->field_progress_comments[$redaction]->set($form_state['values']['answer']);
  }
  else {
    $pt_wrapper->field_progress_comments[NULL]->set($form_state['values']['answer']);
  }
  $pt_wrapper->save();

  foreach ($items as $item) {
    $fc_wrapper = entity_metadata_wrapper('field_collection_item', $item);
    if ($fc_wrapper->field_progress_task_progress->raw() == $pt_wrapper->getIdentifier()) {
      $exists = TRUE;
      $fc_wrapper->field_progress_status->set(DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED);
      $fc_wrapper->save();
      break;
    }
  }

  $pl_wrapper->save();

  $form_state['allow_accept'] = ditoolsi_training_is_lesson_all_finished($lesson,
    $progress->uid);
  $form_state['rebuild']      = TRUE;
}

function ditoolsi_training_accept_task_redaction_submit_ajax(
  $form,
  $form_state
) {
  $commands = array();
  ctools_include('ajax');
  $lesson     = $form_state['build_info']['args'][0];
  $progress   = $form_state['build_info']['args'][2];
  $course_id  = $lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'];
  $commands[] = ctools_ajax_command_redirect("course/" . $course_id . '/' . $lesson->nid . '/' .
    $progress->nid . '/review', 0);

  return array(
    '#type'     => 'ajax',
    '#commands' => $commands,
  );
}

function ditoolsi_training_to_fix_task_redaction_submit($form, &$form_state) {
  $lesson   = $form_state['build_info']['args'][0];
  $task     = $form_state['build_info']['args'][1];
  $progress = $form_state['build_info']['args'][2];
  $account  = user_load($progress->uid);

  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'text_task_progress')
    ->propertyCondition('uid', $progress->uid)
    ->fieldCondition('field_progress_task', 'target_id', $task->nid);

  $result = $query->execute();

  $nid           = key($result['node']);
  $progress_task = node_load($nid);
  $pt_wrapper    = entity_metadata_wrapper('node', $progress_task);
  $pl_wrapper    = entity_metadata_wrapper('node', $progress);

  $items  = $pl_wrapper->field_progress_tasks->value();
  $exists = FALSE;

  $progress_status = ditoolsi_training_get_task_progress($task, $account);
  if ($progress_status == DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED
    || $progress_status == DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST
  ) {
    $redaction = count($pt_wrapper->field_progress_comments) - 1;
    $pt_wrapper->field_progress_comments[$redaction]->set($form_state['values']['answer']);
  }
  else {
    $pt_wrapper->field_progress_comments[NULL]->set($form_state['values']['answer']);
  }
  $pt_wrapper->save();

  foreach ($items as $item) {
    $fc_wrapper = entity_metadata_wrapper('field_collection_item', $item);
    if ($fc_wrapper->field_progress_task_progress->raw() == $pt_wrapper->getIdentifier()) {
      $exists = TRUE;
      $fc_wrapper->field_progress_status->set(DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST);
      $fc_wrapper->save();
      break;
    }
  }

  $pl_wrapper->save();

  $form_state['input']  = array();
  $form_state['values'] = array();

  $form_state['allow_recast'] = TRUE;
  $form_state['rebuild']      = TRUE;
}

function ditoolsi_training_task_redaction_form(
  $form,
  &$form_state,
  $task,
  $redaction
) {
  if ($task->type == 'lesson_text_task') {
    $t_wrapper = entity_metadata_wrapper('node', $task);
    $body      = $t_wrapper->field_text_task_first_redaction->value();
  }
  elseif ($task->type == 'text_task_progress') {
    $t_wrapper = entity_metadata_wrapper('node', $task);
    $body      = $t_wrapper->field_progress_comments[$redaction]->value();
  }
  else {
    return $form;
  }
  $form['#id'] = 'task-redaction-form-' . $task->nid . '-' . $redaction;
  //Get default value
  $redaction_title = $redaction + 1;

  if ($redaction == 'body') {
    $redaction_title = '0';
  }

  $form['redaction'] = array(
    '#type'          => 'text_format',
    '#default_value' => $body['value'],
    '#format'        => 'filtered_html',
    '#title'         => t('Redaction @redaction', array(
        '@redaction' => $redaction_title
      )
    ),
    '#weight'        => 1,
  );

  $form['#after_build'][] = 'ditoolsi_training_task_redaction_form_after_build';

  form_load_include($form_state, 'inc', 'ditoolsi_training', 'includes/form');

  return $form;
}

function ditoolsi_training_task_redaction_form_after_build(
  $form,
  &$form_state
) {
  $form['redaction']['format']['#access'] = FALSE;

  return $form;
}

function ditoolsi_training_autotest_form($form, &$form_state, $lesson, $task) {
  $form['#id']   = 'autotest-form-' . $lesson->nid . '-' . $task->nid;
  $form['#tree'] = TRUE;

  $form['header'] = array(
    '#markup' => '<div class="form-header">' . $task->title . '</div>',
  );

  $form['questions'] = array(
    '#type' => 'container',
  );

  $t_wrapper = entity_metadata_wrapper('node', $task);

  foreach ($t_wrapper->field_autotest_questions->value() as $question) {
    $q_wrapper = entity_metadata_wrapper('field_collection_item', $question);

    $form['questions'][$question->item_id] = array(
      '#title' => $q_wrapper->field_autotest_question->value(),
      '#type'  => 'checkboxes',
    );

    foreach ($q_wrapper->field_autotest_answers->value() as $answer) {
      $a_wrapper                                                           = entity_metadata_wrapper('field_collection_item',
        $answer);
      $form['questions'][$question->item_id]['#options'][$answer->item_id] = $a_wrapper->field_question_answer->value();
    }
  }

  $form['submit'] = array(
    '#type'       => 'submit',
    '#value'      => t('Complete the test'),
    '#submit'     => array('ditoolsi_training_autotest_submit_ajax'),
    '#attributes' => array(
      'class' => array(
        'dit-submit',
      ),
    ),
    '#ajax'       => array(
      'callback' => 'ditoolsi_training_autotest_ajax',
      'wrapper'  => 'autotest-form-' . $lesson->nid . '-' . $task->nid,
      'event'    => 'click',
    ),
  );

  form_load_include($form_state, 'inc', 'ditoolsi_training', 'includes/form');

  return $form;
}

function ditoolsi_training_autotest_submit_ajax($form, &$form_state) {
  global $user;

  $correct_answers = 0;
  $questions       = array();
  $lesson          = $form_state['build_info']['args'][0];
  $task            = $form_state['build_info']['args'][1];

  $t_wrapper = entity_metadata_wrapper('node', $task);

  foreach ($t_wrapper->field_autotest_questions->value() as $question) {
    $q_wrapper                     = entity_metadata_wrapper('field_collection_item',
      $question);
    $questions[$question->item_id] = array(
      'correct' => array(),
      'weight'  => $q_wrapper->field_autotest_question_weight->value(),
    );

    foreach ($q_wrapper->field_autotest_answers->value() as $answer) {
      $a_wrapper = entity_metadata_wrapper('field_collection_item', $answer);

      if ($a_wrapper->field_question_answer_correct->value()) {
        $questions[$question->item_id]['correct'][] = $answer->item_id;
        // continue 2;
      }
    }
  }

  foreach ($form_state['values']['questions'] as $qid => $aid) {
    if (isset($questions[$qid])) {
      $correct_count = 0;
      foreach ($questions[$qid]['correct'] as $value) {
        if ($aid[$value]) {
          $correct_count++;
        }
      }
      if ($correct_count == count($questions[$qid]['correct'])) {
        $correct_answers += $questions[$qid]['weight'];
      }
    }
  }

  if ($t_wrapper->field_autotest_threshold->value() <= $correct_answers) {
    $form_state['storage']['success'] = TRUE;
    ditoolsi_training_finish_autotest($task, $user);
  }
  else {
    $form_state['storage']['success'] = FALSE;
    ditoolsi_training_block_autotest($task, $user);
  }

  $form_state['rebuild'] = TRUE;
}

function ditoolsi_training_autotest_ajax($form, $form_state) {
  global $user;

  $commands = array();
  $lesson   = $form_state['build_info']['args'][0];
  $task     = $form_state['build_info']['args'][1];

  if (empty($form_state['storage']['success'])) {
    $text       = 'Вы не прошли тест. Попробуйте заново.';
    $content    = '<div class="status-text red">' . $text . '</div>';
    $commands[] = ajax_command_replace('#autotest-' . $task->nid,
      '<div id="autotest-' . $task->nid . '">' . $content . '</div>');
  }
  else {
    $content    = '<div class="status-text green">' . t('The test is complete') . '</div>';
    $commands[] = ajax_command_replace('#autotest-' . $task->nid,
      '<div id="autotest-' . $task->nid . '">' . $content . '</div>');

    $is_lesson_finished = ditoolsi_training_lesson_allowed_for_review($lesson,
      $user);

    if ($is_lesson_finished) {
      ctools_include('ajax');
      $commands[] = ctools_ajax_command_reload();
      drupal_set_message(t('You can send a lesson for review'));
    }
  }

  return array(
    '#type'     => 'ajax',
    '#commands' => $commands,
  );
}

function ditoolsi_training_to_fix_task_form(
  $form,
  &$form_state,
  $lesson,
  $task,
  $progress
) {

  $recast = ditoolsi_training_is_lesson_recast($lesson, $progress->uid);

  if ($recast) {
    $form_state['allow_recast'] = TRUE;
  }

  $form['#id'] = 'to-fix-task-form-' . $lesson->nid . '-' . $task->nid;

  $form['to_fix'] = array(
    '#type'   => 'text_format',
    '#format' => 'filtered_html',
    '#title'  => t('To fix'),
    '#weight' => 1,
  );

  $form['submit'] = array(
    '#type'       => 'submit',
    '#value'      => t('To fix'),
    '#submit'     => array('ditoolsi_training_to_fix_task_submit_ajax'),
    '#attributes' => array(
      'class' => array(
        'dit-submit-blue',
      ),
    ),
    '#ajax'       => array(
      'callback' => 'ditoolsi_training_to_fix_task_ajax',
      'wrapper'  => 'to-fix-task-form-' . $lesson->nid . '-' . $task->nid,
      'event'    => 'click',
    ),
    '#weight'     => 2,
  );

  $form['to_accept'] = array(
    '#markup' => l(t('Accept the task'), '', array(
      'attributes' => array(
        'class' => array(
          'toggle-form',
          'accept',
        ),
      ),
    )),
    '#weight' => 2.1,
  );

  $form['#after_build'][] = 'ditoolsi_training_to_fix_task_form_after_build';
  form_load_include($form_state, 'inc', 'ditoolsi_training', 'includes/form');

  return $form;
}

function ditoolsi_training_to_fix_task_form_after_build($form, &$form_state) {
  $form['to_fix']['format']['#access'] = FALSE;

  return $form;
}

function ditoolsi_training_to_fix_task_submit_ajax($form, &$form_state) {
  $lesson   = $form_state['build_info']['args'][0];
  $task     = $form_state['build_info']['args'][1];
  $progress = $form_state['build_info']['args'][2];

  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'text_task_progress')
    ->propertyCondition('uid', $progress->uid)
    ->fieldCondition('field_progress_task', 'target_id', $task->nid);

  $result = $query->execute();

  $nid           = key($result['node']);
  $progress_task = node_load($nid);
  $pt_wrapper    = entity_metadata_wrapper('node', $progress_task);
  $pl_wrapper    = entity_metadata_wrapper('node', $progress);

  $items  = $pl_wrapper->field_progress_tasks->value();
  $exists = FALSE;

  foreach ($items as $item) {
    $fc_wrapper = entity_metadata_wrapper('field_collection_item', $item);

    if ($fc_wrapper->field_progress_task_progress->raw() == $pt_wrapper->getIdentifier()) {
      $exists = TRUE;
      $fc_wrapper->field_progress_status->set(DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST);
      $fc_wrapper->save();
      break;
    }
  }

  if (!empty($form_state['values']['to_fix'])) {
    $pt_wrapper->field_progress_comments[NULL]->set($form_state['values']['to_fix']);
  }

  $pt_wrapper->save();
  $pl_wrapper->save();

  $form_state['input']  = array();
  $form_state['values'] = array();

  $form_state['allow_recast'] = TRUE;
  $form_state['rebuild']      = TRUE;
}

function ditoolsi_training_to_fix_task_ajax($form, $form_state) {
  $lesson              = $form_state['build_info']['args'][0];
  $task                = $form_state['build_info']['args'][1];
  $current_task_number = NULL;
  $key                 = 1;

  foreach ($lesson->tasks as $nid => $task_item) {
    if ($task->nid == $task_item->nid) {
      $current_task_number = ++$key;
      break;
    }

    $key++;
  }

  $commands   = array();
  $commands[] = array(
    'command' => 'ditoolsiTrainingSwitchTask',
    'task'    => $current_task_number,
    'class'   => 'status-recast',
  );

  return array(
    '#type'     => 'ajax',
    '#commands' => $commands,
  );
}

function ditoolsi_training_get_transfer_lesson_form($lesson, $progress) {
  $finished = 0;
  $recast   = 0;
  $review   = 0;
  $form     = '';

  $task_statuses = ditoolsi_training_get_task_status_for_review($lesson,
    $progress);
  $tasks_count   = count($task_statuses);

  foreach ($task_statuses as $value) {
    switch ($value) {
      case DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST:
        $recast++;
        break;

      case DITOOLSI_TRAINING_PROGRESS_STATUS_REVIEW:
        $review++;
        break;

      case DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED:
        $finished++;
        break;
    }
  }

  if ($finished == $tasks_count) {
    if (!empty($lesson->field_lesson_final[LANGUAGE_NONE][0]['value'])) {
      $course_id = $lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'];
      $node      = node_load($course_id);

      if (!empty($node->field_next_course[LANGUAGE_NONE][0]['target_id'])) {
        $next_course = $node->field_next_course[LANGUAGE_NONE][0]['target_id'];
        $form        = drupal_get_form('ditoolsi_training_transfer_next_course_form',
          $lesson, $progress);
      }
      else {
        $form = drupal_get_form('ditoolsi_training_transfer_finish_course_form',
          $lesson, $progress);
      }
    }
    else {
      $form = drupal_get_form('ditoolsi_training_transfer_next_lesson_form',
        $lesson, $progress);

    }
  }
  elseif (($finished + $recast) == $tasks_count) {
    $form = drupal_get_form('ditoolsi_training_transfer_recast_lesson_form',
      $lesson, $progress);
  }

  return $form;
}

function ditoolsi_training_get_task_status_for_review($lesson, $progress) {
  $task_statuses = array();
  $tasks_nid     = array_keys($lesson->tasks);

  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'text_task_progress')
    ->propertyCondition('uid', $progress->uid)
    ->fieldCondition('field_progress_task', 'target_id', $tasks_nid, 'IN');

  $result = $query->execute();

  if ($result) {
    $nids           = array_keys($result['node']);
    $tasks_count    = count($nids);
    $progress_tasks = node_load_multiple($nids);
    $pl_wrapper     = entity_metadata_wrapper('node', $progress);
    $items          = $pl_wrapper->field_progress_tasks->value();

    foreach ($progress_tasks as $progress_task) {
      $pt_wrapper = entity_metadata_wrapper('node', $progress_task);
      foreach ($items as $item) {
        $fc_wrapper = entity_metadata_wrapper('field_collection_item', $item);
        if ($fc_wrapper->field_progress_task_progress->raw() == $pt_wrapper->getIdentifier()) {
          $status                    = $fc_wrapper->field_progress_status->value();
          $task                      = $pt_wrapper->field_progress_task->value();
          $task_statuses[$task->nid] = $status;
          break;
        }
      }
    }
  }


  return $task_statuses;
}

function ditoolsi_training_transfer_next_lesson_form(
  $form,
  &$form_state,
  $lesson,
  $progress
) {
  $form['recast'] = array(
    '#type'       => 'submit',
    '#value'      => t('Transfer to the next lesson'),
    '#weight'     => 4,
    '#attributes' => array(
      'class' => array(
        'dit-submit',
        'accept-lesson',
      ),
    ),
    '#id'         => 'lesson-accept-' . $lesson->nid,
    '#submit'     => array('ditoolsi_training_accept_lesson_submit_ajax'),
    '#ajax'       => array(
      'callback' => 'ditoolsi_training_accept_lesson_ajax',
      'event'    => 'click',
    ),
  );

  form_load_include($form_state, 'inc', 'ditoolsi_training', 'includes/form');

  return $form;
}

function ditoolsi_training_accept_lesson_submit_ajax($form, &$form_state) {
  $form_state['success'] = TRUE;
}

function ditoolsi_training_accept_lesson_ajax($form, $form_state) {
  $commands = array();
  ctools_include('ajax');
  $lesson    = $form_state['build_info']['args'][0];
  $progress  = $form_state['build_info']['args'][1];
  $course_id = $lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'];
  $form      = drupal_get_form('ditoolsi_training_accept_lesson_popup_form',
    $lesson, $progress);
  $html      = '<div id="register-popup" class="show">' . render($form) . '<div id="ditoolsi-overlay"></div></div>';

  $commands[] = ajax_command_remove('#register-popup');
  $commands[] = ajax_command_append('body', '<div id="register-popup"></div>');
  $commands[] = ajax_command_replace('#register-popup', $html);

  return array(
    '#type'     => 'ajax',
    '#commands' => $commands,
  );
}

function ditoolsi_training_accept_lesson_popup_form(
  $form,
  &$form_state,
  $lesson,
  $progress
) {
  global $user;

  $marks       = array();
  $course      = node_load($lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id']);
  $rating_type = field_get_items('node', $course, 'field_course_rating_type');

  $query = db_select('node', 'n');
  $query->join('field_data_field_progress_lesson_id', 'pli',
    'pli.entity_id = n.nid');
  $query->join('field_data_field_lesson_progress_message', 'pm',
    'pm.entity_id = n.nid');
  $query
    ->fields('n', array('nid'))
    ->fields('pm', array('field_lesson_progress_message_value'))
    ->condition('pli.field_progress_lesson_id_target_id', $lesson->nid)
    ->condition('n.type', 'lesson_progress');

  $messages = array(0 => t('No use')) + $query->execute()->fetchAllKeyed(0, 1);

  if (isset($rating_type[0]['value']) && $rating_type[0]['value'] == DT_TRAINING_RATING_2) {
    $form['mark'] = array(
      '#type'         => 'checkbox',
      '#title'        => t('Passed'),
      '#return_value' => 1,
      '#value'        => 1,
      '#access'       => FALSE,
    );
  }
  else {
    for ($i = 0; $i < 6; $i++) {
      $marks[$i] = $i;
    }

    $form['mark'] = array(
      '#type'    => 'select',
      '#title'   => t('Mark'),
      '#options' => $marks,
    );
  }

  $json = json_encode($messages);

  foreach ($messages as $key => &$message) {
    $message = strip_tags($message);
    $message = trim($message);

    if (!$message) {
      unset($messages[$key]);
      continue;
    }

    $alter = array(
      'max_length'    => 60,
      'ellipsis'      => TRUE,
      'word_boundary' => FALSE,
      'html'          => FALSE,
    );

    $message = views_trim_text($alter, $message);
  }

  if (count($messages) > 1) {
    $form['template'] = array(
      '#type'       => 'select',
      '#options'    => $messages,
      '#title'      => t('Use ready Comment'),
      '#attributes' => array(
        'data-json' => $json,
      ),
    );
  }

  $form['comment'] = array(
    '#type'  => 'textarea',
    '#title' => t('Comment'),
  );

  $access_token = isset($user->data['_vk_access_token']) ? $user->data['_vk_access_token'] : '';
  $account      = ditoolsi_profile_account($progress);

  if ($access_token && field_get_items('user', $account,
      'field_vk_page') && user_access('training notify vk')
  ) {
    // $form['append_comment'] = array(
    //   '#type' => 'checkbox',
    //   '#title' => 'Добавить комментарий в сообщение',
    //   '#return_value' => 1,
    //   '#default_value' => 1,
    // );
  }

  // if (defined('DT_TRAINING_PROMO_COURSE_' . $course_id)) {
  //   $form['send_vk'] = array(
  //     '#type' => 'checkbox',
  //     '#title' => 'Отправить сообщение Вконтакте',
  //     '#return_value' => 1,
  //   );
  // }

  // $current_lesson = db_select('ditoolsi_training_pupils_list', 'pl')
  //   ->fields('pl', array('current_lesson'))
  //   ->condition('pl.user_id', $progress->uid)
  //   ->condition('pl.course_id', $lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'])
  //   ->execute()
  //   ->fetchField();

  $form['current_lesson'] = array(
    '#type'  => 'hidden',
    '#value' => $lesson->field_lesson_number[LANGUAGE_NONE][0]['value'],
  );

  $form['submit'] = array(
    '#type'   => 'submit',
    '#value'  => t('Send comment'),
    '#submit' => array('ditoolsi_training_accept_lesson_popup_form_submit'),
    '#ajax'   => array(
      'callback' => 'ditoolsi_training_accept_lesson_popup_form_ajax',
      'event'    => 'click',
    ),
  );

  form_load_include($form_state, 'inc', 'ditoolsi_training', 'includes/form');
  form_load_include($form_state, 'php', 'ditoolsi_training',
    'includes/HttpTools.class');

  return $form;
}

function ditoolsi_training_accept_lesson_popup_form_ajax($form, $form_state) {
  if (!empty($form_state['build_info']['args'])
    && $form_state['success']
  ) {
    $lesson    = $form_state['build_info']['args'][0];
    $course_id = $lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'];
    $commands  = array();
    ctools_include('ajax');
    $commands[] = ctools_ajax_command_redirect("courses/requests-review", 0,
      array('query' => array('course' => $course_id)));

    return array(
      '#type'     => 'ajax',
      '#commands' => $commands,
    );
  }
}

function ditoolsi_training_accept_lesson_popup_form_submit(
  $form,
  &$form_state
) {
  global $user;

  $progress                                                   = $form_state['build_info']['args'][1];
  $progress->field_progress_status[LANGUAGE_NONE][0]['value'] = DITOOLSI_TRAINING_PROGRESS_STATUS_FINISHED;
  node_save($progress);
  $lesson  = $form_state['build_info']['args'][0];
  $course  = node_load($lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id']);
  $account = user_load($progress->uid);

  $flag = flag_get_user_flags('node', $lesson->nid, $progress->uid,
    $sid = 'student_next_lesson');

  if (!isset($flag['student_next_lesson'])) {
    $flag_finish = flag('flag', 'student_next_lesson', $lesson->nid, $progress,
      TRUE);
  }

  $course_id    = $lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'];
  $bonus_points = $form_state['values']['mark'];

  db_update('ditoolsi_training_pupils_list')
    ->fields(array(
      'current_lesson' => ($form_state['values']['current_lesson'] + 1),
    ))
    ->condition('user_id', $progress->uid)
    ->condition('course_id', $course_id)
    ->execute();

  $pl_wrapper = entity_metadata_wrapper('node', $progress);
  $pl_wrapper->field_lesson_progress_mark->set($bonus_points);
  $pl_wrapper->field_lesson_progress_message->set($form_state['values']['comment']);
  $pl_wrapper->field_progress_author_uid->set($user->uid);

  $pl_wrapper->save();

  if (module_exists('ditoolsi_bonuses')) {
    //Create 'Bonus points' node
    $node = entity_create('node', array(
      'type' => 'bonus_points',
      'uid'  => $user->uid,
    ));

    $title                             = t('Bonus points for completion lesson !lesson',
      array('!lesson' => $lesson->title,));
    $bp_wrapper                        = entity_metadata_wrapper('node', $node);
    $bp_wrapper->title                 = strip_tags($title);
    $bp_wrapper->field_charging_lesson = $lesson->nid;
    $bp_wrapper->field_charging_pupil  = $progress->uid;
    $bp_wrapper->field_charging_points = $bonus_points;
    $bp_wrapper->save();

    //Edit user's bonus account
    ditoolsi_bonuses_upd_course_bonus_balance($account, $course_id,
      $bonus_points);
  }

  $comment = strip_tags($form_state['values']['comment']);
  $comment = trim($comment);

  if ($comment) {
    ditoolsi_training_create_private_msg($user, $account,
      $form_state['values']['comment']);
  }

  $query = db_select('node', 'n');
  $query->join('field_data_field_lesson_number', 'ln', 'ln.entity_id = n.nid');
  $query->join('field_data_field_lesson_course', 'lc', 'lc.entity_id = n.nid');
  $query
    ->fields('n', array('nid'))
    ->condition('ln.field_lesson_number_value',
      ($form_state['values']['current_lesson'] + 1))
    ->condition('lc.field_lesson_course_target_id', $course->nid)
    ->range(0, 1);

  $old_lesson_title = "<b>{$lesson->title}</b>";

  $nid = $query->execute()->fetchField();
// x3 зачем эта проверка и переопределение $lesson?  если nid false - то $lesson из аргумента
//  $lesson       = $nid ? node_load($nid) : NULL;
  $lesson       = $nid ? node_load($nid) : $lesson;
  $access_token = isset($user->data['_vk_access_token']) ? $user->data['_vk_access_token'] : '';
  $stop         = $access_token && $comment && ($field_vk = field_get_items('user',
      $account,
      'field_vk_page')) && user_access('training notify vk') && $lesson;
  watchdog('vk_notify_access_result', $stop);
  /*Отправка писем*/
  mgc_send_mail_notification($bonus_points, $course, $account, $node, $comment);
  /*Если куратор - то использовать доступ к ВК от учителя*/
  if (mgc_user_has_curator_role()) {
    $teacher_uid     = $course->uid;
    $teacher_profile = user_load($teacher_uid);
    //$teacher_access_token = $teacher_profile->data['_vk_access_token'];
    $teacher_access_token = isset($teacher_profile->data['_vk_access_token']) ? $teacher_profile->data['_vk_access_token'] : '';
    $access_token         = $teacher_access_token;
  }
  if ($access_token && ($access_token != '') && $comment && ($field_vk = field_get_items('user',
      $account,
      'field_vk_page')) && user_access('training notify vk') && $lesson
  ) {
    $parts = parse_url($field_vk[0]['value']);

    if ($bonus_points) {
      $vk_notify_next = isset($course->field_course_vk_notify_next_less['und'][0]['value']) ? $course->field_course_vk_notify_next_less['und'][0]['value'] : '';
      $vk_notify_next = token_replace($vk_notify_next, array(
        'node' => $node,
        'user' => $account,
      ));
      $vk_notify_next = format_string($vk_notify_next, array(
        '@points'      => $bonus_points,
        '@lesson_link' => url("course/{$course->nid}/{$lesson->nid}",
          array('absolute' => TRUE)),
      ));
      $comment .= $vk_notify_next;

    }
    else {

      $vk_notify_credit = isset($course->field_course_vk_notify_credit['und'][0]['value']) ? $course->field_course_vk_notify_credit['und'][0]['value'] : '';
      $vk_notify_credit = token_replace($vk_notify_credit, array(
        'node' => $node,
        'user' => $account,
      ));
      $vk_notify_credit = format_string($vk_notify_credit, array(
        '@points'      => $bonus_points,
        '@lesson_link' => url("course/{$course->nid}/{$lesson->nid}",
          array('absolute' => TRUE)),
      ));
      $comment .= $vk_notify_credit;
    }

    if (isset($parts['path'])) {
      $parts['path'] = ltrim($parts['path'], '/');

      $comment = token_replace($comment, array(
        'node' => $course,
        'user' => $account,
      ));

      if (!empty($course->field_course_vk_notify_next_less[LANGUAGE_NONE][0]['value'])) {
        $comment = "Комментарий куратора: \n" . $comment;
        $comment = $course->field_course_vk_notify_next_less[LANGUAGE_NONE][0]['value'] . "\n\n" . $comment;
        $comment = format_string($comment, array(
          '@points'      => $bonus_points,
          '@lesson_link' => url("course/{$course->nid}/{$lesson->nid}",
            array('absolute' => TRUE)),
        ));
      }
      $stop = 'Stop';
      if ($access_token != NULL) {
        db_insert('ditoolsi_training_vk_messages_queue')
          ->fields(array(
            'comment'      => $comment,
            'vk_id'        => $parts['path'],
            'access_token' => $access_token,
          ))
          ->execute();
      }

    }
  }

  // $notification = entity_create('node', array(
  //   'type' => 'notification',
  //   'uid' => $user->uid,
  //   'status' => NODE_PUBLISHED,
  //   'title' => t('Notification for user [uid:@uid]', array(
  //     '@uid' => $account->uid,
  //   )),
  // ));
  // $n_wrapper = entity_metadata_wrapper('node', $notification);
  // $n_wrapper->field_notification_type->set(DITOOLSI_TRAINING_TT_TRANSFER);
  // $n_wrapper->field_notification_course->set($course_id);
  // $n_wrapper->save();

  //notification rule
  $notification_options            = array();
  $course                          = node_load($course_id);
  $term                            = taxonomy_term_load(DT_TRAINING_TT_TRANSFER);
  $account                         = user_load($progress->uid);
  $notification_options['course']  = $course;
  $notification_options['account'] = $account;
  $notification_options['term']    = $term;

  rules_invoke_event('ditoolsi_training_pupils_notification',
    $notification_options);

  if ($bonus_points > 0) {
    ditoolsi_set_account_message($account,
      t('The teacher has put you an estimate made for @rate lesson', array(
        '@rate' => $bonus_points,
      )));
  }
  $stop = 'Stop';
  if ($lesson) {
    $stop = 'Stop';

    dt_training_load_lesson_progress($lesson, $account);
    $name         = '<b>' . ditoolsi_profile_name($user) . '</b>';
    $lesson_title = l($lesson->title, "course/{$course->nid}/{$lesson->nid}");
    $course_title = l($course->title, "course/{$course->nid}");
    $rating_type  = field_get_items('node', $course,
      'field_course_rating_type');

    if (isset($rating_type[0]['value']) && $rating_type[0]['value'] == DT_TRAINING_RATING_2) {
      $rate = $bonus_points ? 'зачет' : 'не зачет';
    }
    else {
      $rate = 'оценку ' . $bonus_points;
    }

    ditoolsi_add_notification($account,
      $name . ' поставил вам ' . $rate . ' по уроку ' . $old_lesson_title . ' и перевел вас на следующий урок ' .
      $lesson_title . ' курса ' . $course_title . '. Перейдите по ссылке с названием следующего урока, что бы посмотреть материалы.');
    global $user;
    $teacher         = user_load($user->uid);
    $subject_curator = "{$account->mail} выслал задания на проверку";
    $fio_first_name  = isset($account->field_first_name['und'][0]['value']) ? $account->field_first_name['und'][0]['value'] : '';
    $fio_middle_name = isset($account->field_middle_name['und'][0]['value']) ? $account->field_middle_name['und'][0]['value'] : '';
    $fio_last_name   = isset($account->field_last_name['und'][0]['value']) ? $account->field_last_name['und'][0]['value'] : '';
    $fio             = "{$fio_first_name} {$fio_middle_name} {$fio_last_name}";
    $message_curator = 'Вам отправлено задание на проверку ' . $lesson_title . '  от ученика ' . $user->mail . ' ' . $fio . ' из курса ' . $course_title;
    $message_curator .= '<br><br>Перейти к проверке можете по ссылке <a href="http://rewrite.up-lvl.ru/user">http://rewrite.up-lvl.ru/user</a>';
    $stop = 'Stop';
    /*Если это не бихис bi1811@yandex.ru*/
    if($teacher->uid != 31275) {
      drupal_mail('system', 'mail', $teacher->mail,
        language_default(), array(
          'context' => array(
            'subject' => $subject_curator,
            'message' => $message_curator,
          )
        ));
    }
  }
  $form_state['success'] = TRUE;
}

function ditoolsi_training_transfer_next_course_form(
  $form,
  &$form_state,
  $lesson,
  $progress
) {
  $form['recast'] = array(
    '#type'       => 'submit',
    '#value'      => t('Transfer to the next course'),
    '#weight'     => 2.2,
    '#attributes' => array(
      'class' => array(
        'dit-submit',
        'accept-course',
      ),
    ),
    '#id'         => 'course-accept',
    '#submit'     => array('ditoolsi_training_accept_course_submit_ajax'),
  );

  form_load_include($form_state, 'inc', 'ditoolsi_training', 'includes/form');

  return $form;
}

function ditoolsi_training_accept_course_submit_ajax($form, &$form_state) {
  $progress  = $form_state['build_info']['args'][1];
  $lesson    = $form_state['build_info']['args'][0];
  $course_id = $lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'];
  $node      = node_load($course_id);

  $account      = ditoolsi_profile_account($progress);
  $course_title = l($node->title, "course/{$node->nid}");
  ditoolsi_add_notification($account,
    'Поздравляем! Вы успешно выполнили все задания курса ' . $course_title . ' и закончили обучение.');

  if (isset($node->field_next_course[LANGUAGE_NONE][0]['target_id'])) {
    $next_course = $node->field_next_course[LANGUAGE_NONE][0]['target_id'];
    db_update('ditoolsi_training_pupils_list')
      ->expression('course_id', $next_course)
      ->expression('current_lesson', '1')
      ->condition('user_id', $progress->uid)
      ->condition('course_id', $course_id)
      ->execute();
  }

  $form_state['redirect'] = 'courses/requests-review';
}

function ditoolsi_training_transfer_finish_course_form(
  $form,
  &$form_state,
  $lesson,
  $progress
) {
  $form['recast'] = array(
    '#type'       => 'submit',
    '#value'      => t('Complete lesson'),
    '#weight'     => 2.2,
    '#attributes' => array(
      'class' => array(
        'dit-submit',
        'finish-lesson',
      ),
    ),
    '#id'         => 'finish-lesson',
    '#submit'     => array('ditoolsi_training_accept_lesson_submit_ajax'),
    '#ajax'       => array(
      'callback' => 'ditoolsi_training_accept_lesson_ajax',
      'event'    => 'click',
    ),
  );

  form_load_include($form_state, 'inc', 'ditoolsi_training', 'includes/form');

  return $form;
}

function ditoolsi_training_transfer_recast_lesson_form(
  $form,
  &$form_state,
  $lesson,
  $progress
) {
  $form['recast'] = array(
    '#type'       => 'submit',
    '#value'      => t('Return a lesson back for revision'),
    '#weight'     => 2.2,
    '#attributes' => array(
      'class' => array(
        'dit-submit',
        'return-to-revision',
      ),
    ),

    '#id'     => 'lesson-to-fix',
    '#submit' => array('ditoolsi_training_to_fix_lesson_submit_ajax'),
    '#ajax'   => array(
      'callback' => 'ditoolsi_training_to_fix_lesson_ajax',
      'event'    => 'click',
    ),
  );

  form_load_include($form_state, 'inc', 'ditoolsi_training', 'includes/form');

  return $form;
}

function ditoolsi_training_to_fix_lesson_submit_ajax($form, &$form_state) {
  global $user;

  $lesson                                                         = $form_state['build_info']['args'][0];
  $course_id                                                      = $lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'];
  $progress                                                       = $form_state['build_info']['args'][1];
  $progress->field_progress_status[LANGUAGE_NONE][0]['value']     = DITOOLSI_TRAINING_PROGRESS_STATUS_RECAST;
  $progress->field_progress_author_uid[LANGUAGE_NONE][0]['value'] = $user->uid;

  node_save($progress);
  $form_state['success'] = TRUE;

  //notification rule
  $notification_options            = array();
  $course                          = node_load($course_id);
  $term                            = taxonomy_term_load(DT_TRAINING_TT_RECAST);
  $account                         = user_load($progress->uid);
  $notification_options['course']  = $course;
  $notification_options['account'] = $account;
  $notification_options['term']    = $term;

  rules_invoke_event('ditoolsi_training_pupils_notification',
    $notification_options);

  if (($field_vk_page = field_get_items('user', $account,
      'field_vk_page')) && user_access('training notify vk')
  ) {
    $access_token = isset($user->data['_vk_access_token']) ? $user->data['_vk_access_token'] : '';

    if (isset($course->field_course_vk_notify_recast[LANGUAGE_NONE][0]['value'])) {
      $message = $course->field_course_vk_notify_recast[LANGUAGE_NONE][0]['value'];
    }

    if (!empty($access_token) && !empty($message)) {
      $message = token_replace($message, array(
        'node' => $course,
        'user' => $account,
      ));

      $message = format_string($message, array(
        '@lesson_link' => url("course/{$course->nid}/{$lesson->nid}",
          array('absolute' => TRUE)),
      ));

      db_insert('ditoolsi_training_vk_messages_queue')
        ->fields(array(
          'comment'      => $message,
          'vk_id'        => $field_vk_page[0]['value'],
          'access_token' => $access_token,
        ))
        ->execute();
    }
  }

  $name         = '<b>' . ditoolsi_profile_name($user) . '</b>';
  $lesson_title = l($lesson->title, "course/{$course_id}/{$lesson->nid}");
  $course_title = l($course->title, "course/{$course->nid}");
  ditoolsi_add_notification($account,
    $name . ' поставил не зачет и вернул на доработку задания урока ' . $lesson_title . ' курса. Нажмите на название урока, что бы перейти к исправлению задания.');
  $message = isset($course->field_course_vk_notify_recast['und'][0]['value']) ? $course->field_course_vk_notify_recast['und'][0]['value'] : '';
  $message = token_replace($message, array(
    'node' => $course,
    'user' => $account,
  ));

  $message = format_string($message, array(
    '@lesson_link' => url("course/{$course->nid}/{$lesson->nid}",
      array('absolute' => TRUE)),
  ));

  $mail_subject = $name . ' поставил не зачет и вернул на доработку задания урока ' . $lesson->title;
  global $language;
  drupal_mail('system', __FUNCTION__, $account->mail, $language, array(
    'context' => array(
      'subject' => $mail_subject,
      'message' => $message,
    ),
  ));

}

function ditoolsi_training_to_fix_lesson_ajax($form, $form_state) {
  if (!empty($form_state['success'])) {
    $commands = array();
    ctools_include('ajax');
    $lesson     = $form_state['build_info']['args'][0];
    $course_id  = $lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'];
    $commands[] = ctools_ajax_command_redirect("courses/requests-review", 0,
      array('query' => array('course' => $course_id)));

    return array(
      '#type'     => 'ajax',
      '#commands' => $commands,
    );
  }
}

function ditoolsi_training_buy_course_form($form, &$form_state, $course) {
  $form['submit'] = array(
    '#type'       => 'submit',
    '#value'      => t('Buy'),
    '#attributes' => array(
      'class' => array(
        'dit-submit',
      ),
    ),
  );

  $form['cancel'] = array(
    '#type'  => 'link',
    '#title' => t('Cancel'),
    '#href'  => 'course/' . $course->nid,
  );

  return $form;
}

function ditoolsi_training_buy_course_form_submit($form, &$form_state) {
  global $user;

  $course  = $form_state['build_info']['args'][0];
  $cost    = $course->field_course_cost[LANGUAGE_NONE][0]['value'];
  $balance = db_select('dt_payment_accounts', 'pa')
    ->fields('pa', array('balance'))
    ->condition('pa.uid', $user->uid)
    ->execute()
    ->fetchField();

  if ($balance < $cost) {
    drupal_set_message(t('Insufficient funds in the account'), 'error');
    drupal_set_message(l(t('Add funds'), 'balance'), 'error');
  }
  else {
    drupal_set_message(t('Now you can learn in this course'));
    db_update('ditoolsi_payment_accounts')
      ->expression('balance', 'balance - :cost', array(
        ':cost' => $cost,
      ))
      ->condition('uid', $user->uid)
      ->execute();
    db_merge('ditoolsi_training_pupils_list')
      ->fields(array(
        'course_id'      => $course->nid,
        'user_id'        => $user->uid,
        'current_lesson' => 1,
        'created'        => REQUEST_TIME,
      ))
      ->key(array(
        'course_id' => $course->nid,
        'user_id'   => $user->uid,
      ))
      ->execute();
  }
  $form_state['redirect'] = 'course/' . $course->nid;
}

function ditoolsi_training_add_pupils_form($form, &$form_state, $course) {
  $form['#id']         = 'add-pupils-to-lesson';
  $form['#attributes'] = array(
    'class' => array(
      'process-ditools-labels',
    ),
  );

  $form['names'] = array(
    '#tree'   => TRUE,
    '#prefix' => '<div id="names-wrapper">',
    '#suffix' => '</div>',
  );

  $form['names']['course_title'] = array(
    '#type'   => 'item',
    '#title'  => $course->title,
    '#prefix' => '<div class="course-title">',
    '#suffix' => '</div>',
  );

  $form['names']['title'] = array(
    '#type'  => 'item',
    '#title' => t('Add pupil manually'),
  );

  if (empty($form_state['storage']['course_id'])) {
    $form_state['storage']['course_id'] = $course->nid;
  }

  // See example_add_more_form_add().
  if (empty($form_state['storage']['name_count'])) {
    $form_state['storage']['name_count'] = 1;
  }

  $select = db_select('field_data_field_lesson_course', 'field_lesson_course');
  $select->fields('field_lesson_course', array('entity_id'));
  $select->condition('field_lesson_course.field_lesson_course_target_id',
    $course->nid);
  $results    = $select->execute()->fetchAll();
  $list       = array();
  $lesson_ids = array();

  foreach ($results as $result_value) {
    $lesson_ids[] = $result_value->entity_id;
  }

  $lessons = node_load_multiple($lesson_ids);

  foreach ($lessons as $lesson) {
    $list[$lesson->field_lesson_number[LANGUAGE_NONE][0]['value']] = $lesson->field_lesson_number[LANGUAGE_NONE][0]['value'] . ' | ' . $lesson->title;
  }

  for ($i = 0; $i < $form_state['storage']['name_count']; $i++) {
    $class = $i % 2 == 1 ? 'odd' : 'even';

    $form['names'][$i]['name'] = array(
      '#prefix' => "<div class='fields fields-{$i} {$class}'>",
      '#title'  => t('E-mail'),
      '#type'   => 'textfield',
    );

    $form['names'][$i]['lesson'] = array(
      '#type'          => 'select',
      '#options'       => $list,
      '#default_value' => 1,
      '#suffix'        => '</div>'
    );
  }

  $form['add_more'] = array(
    '#attributes' => array(
      'class' => array(
        'add-field-pupils'
      ),
    ),
    '#type'       => 'submit',
    '#value'      => t('Add fields'),
    '#submit'     => array('ditoolsi_training_form_add_more_submit'),
    '#ajax'       => array(
      'wrapper'  => 'names-wrapper',
      'callback' => 'ditoolsi_training_form_update_ajax',
      'event'    => 'click',
    ),
  );

  $form['#attached'] = array(
    'css' => array(
      drupal_get_path('theme', 'bootstrap_subtheme') . '/css/add-pupils.css',
    ),
  );

  $form['submit'] = array(
    '#attributes' => array(
      'class' => array(
        'load-pupils'
      ),
    ),
    '#type'       => 'submit',
    '#value'      => t('Load data'),
    '#ajax'       => array(
      'callback' => 'ditoolsi_training_add_pupils_ajax',
      'wrapper'  => 'add-pupils-to-lesson',
      'effect'   => 'fade',
      'event'    => 'click',
    ),
  );

  return $form;
}

function ditoolsi_training_add_pupils_ajax($form, $form_state) {
  global $user, $language;

  $name         = '<b>' . ditoolsi_profile_name($user) . '</b>';
  $course       = node_load($form_state['storage']['course_id']);
  $course_title = l($course->title, "course/{$course->nid}");

  foreach ($form_state['values']['names'] as $key => $value) {
    if (!empty($value['name'])) {
      if (valid_email_address($value['name'])) {
        $account = user_load_by_mail($value['name']);
        if (!empty($account)) {
          $query = db_select('ditoolsi_training_pupils_list',
            'training_pupils_list');
          $query->fields('training_pupils_list', array('id'));
          $query->condition('training_pupils_list.user_id', $account->uid);
          $query->condition('training_pupils_list.course_id',
            $form_state['storage']['course_id']);
          $results = $query->execute()->fetchAll();
          if (!empty($results)) {
            drupal_set_message(t('Pupil @user is already on your course',
              array('@user' => $account->mail)), 'error');
          }
          else {
            $query = db_select('node', 'n');
            $query->join('field_data_field_lesson_number', 'ln',
              'ln.entity_id = n.nid');
            $query->join('field_data_field_lesson_course', 'lc',
              'lc.entity_id = n.nid');
            $query
              ->fields('n', array('nid'))
              ->condition('ln.field_lesson_number_value', $value['lesson'])
              ->condition('lc.field_lesson_course_target_id', $course->nid)
              ->range(0, 1);

            $nid    = $query->execute()->fetchField();
            $lesson = node_load($nid);

            if ($lesson) {
              dt_training_update_start_time_progress($lesson, $account);
              $lesson_title = l($lesson->title,
                "course/{$course->nid}/{$lesson->nid}");
              $text         = $name . ' открыл вам доступ к курсу ' . $course_title . '. Вам доступен первый урок ' . $lesson_title;
              ditoolsi_add_notification($account, $text);
            }

            db_insert('ditoolsi_training_pupils_list')
              ->fields(array(
                'course_id'      => $form_state['storage']['course_id'],
                'user_id'        => $account->uid,
                'current_lesson' => $value['lesson'],
                'created'        => REQUEST_TIME,
                'status'         => DITOOLSI_TRAINING_PUPIL_ACTIVE,
              ))
              ->execute();
            drupal_set_message(t('Pupil @user successfully added to your course',
              array('@user' => $account->mail)));
            $mail               = ditoolsi_training_get_mail_activation($course);
            $mail[1]['body']    = token_replace($mail[1]['body'], array(
              'node' => $course,
              'user' => $account,
            ));
            $mail[1]['subject'] = token_replace($mail[1]['subject'], array(
              'node' => $course,
              'user' => $account,
            ));
            $stop               = 'Stop';

            //mgc_send_mail($account->mail, $subject, $mail_body);
            /*для препода на Профологии нужно сделать, чтобы письма на меил о приглашениях на платформу приходили от их имени "Навстречу стилю"*/
            if ($user->uid == 32458) {
              $email_from = 'navstrechu_stilju@up-lvl.ru';
              drupal_mail('system', __FUNCTION__, $account->mail, $language,
                array(
                  'context' => array(
                    'subject' => $mail[1]['subject'],
                    'message' => $mail[1]['body'],
                    //'message' => 'fffffff',
                  ),

                ),
                $email_from);
            }
            elseif ($user->uid == 32405) {
              $email_from = 'dmitriylomot@up-lvl.ru';
              drupal_mail('system', __FUNCTION__, $account->mail, $language,
                array(
                  'context' => array(
                    'subject' => $mail[1]['subject'],
                    'message' => $mail[1]['body'],
                    //'message' => 'fffffff',
                  ),

                ),
                $email_from);
            }
            else {
              drupal_mail('system', __FUNCTION__, $account->mail, $language,
                array(
                  'context' => array(
                    'subject' => $mail[1]['subject'],
                    'message' => $mail[1]['body'],
                  ),

                ));
            }

          }
        }
        else {
          $password = user_password();
          $roles    = user_roles();
          $fields   = array(
            'name'   => $value['name'],
            'mail'   => $value['name'],
            'pass'   => DITOOLSI_DEFAULT_PASS,
            'status' => 1,
            'init'   => $value['name'],
            'access' => time(),
            'roles'  => array(
              DITOOLSI_PUPIL_RID => $roles[DITOOLSI_PUPIL_RID]
            ),
          );

          $account = user_save(NULL, $fields);
          $query   = db_select('node', 'n');
          $query->join('field_data_field_lesson_number', 'ln',
            'ln.entity_id = n.nid');
          $query->join('field_data_field_lesson_course', 'lc',
            'lc.entity_id = n.nid');
          $query
            ->fields('n', array('nid'))
            ->condition('ln.field_lesson_number_value', $value['lesson'])
            ->condition('lc.field_lesson_course_target_id', $course->nid)
            ->range(0, 1);

          $nid    = $query->execute()->fetchField();
          $lesson = node_load($nid);

          if ($lesson) {
            dt_training_update_start_time_progress($lesson, $account);
            $lesson_title = l($lesson->title,
              "course/{$course->nid}/{$lesson->nid}");
            $text         = $name . ' открыл вам доступ к курсу ' . $course_title . '. Вам доступен первый урок ' . $lesson_title;
            ditoolsi_add_notification($account, $text);
          }

          $mail               = ditoolsi_training_get_mail_activation($course);
          $mail[0]['subject'] = token_replace($mail[0]['subject'], array(
            'node' => $course,
            'user' => $account,
          ));
          $mail[0]['body']    = token_replace($mail[0]['body'], array(
            'node' => $course,
            'user' => $account,
          ));
          $mail[1]['body']    = token_replace($mail[1]['body'], array(
            'node' => $course,
            'user' => $account,
          ));
          $mail_body          = $mail[0]['body'] . '<br>
         Для входа используйте следующие данные:         
         Логин: <b>' . $fields['name'] . '</b> пароль:<b>' . $fields['pass'] . '</b>' .
            '<br>';
//            '<br><br>* <em>Обязательно поменяйте стандартный пароль на свой!</em><br>';
          $mail_body .= '<br>=======================================<br><br>' . $mail[1]['body'];
          /*mgc_send_mail($account->mail, $mail[0]['subject'], $mail[0]['body']);
*/          /*для препода на Профологии нужно сделать, чтобы письма на меил о приглашениях на платформу приходили от их имени "Навстречу стилю"*/
          if ($user->uid == 32458) {
            drupal_mail('system', __FUNCTION__, $value['name'], $language,
              array(
                'context' => array(
                  'subject' => $mail[0]['subject'],
                  'message' => $mail[0]['body'],
                ),

                'account'  => $account,
                'password' => $password,
              )
              , 'navstrechu_stilju@up-lvl.ru');
          }
          elseif ($user->uid == 32405) {
            drupal_mail('system', __FUNCTION__, $value['name'], $language,
              array(
                'context' => array(
                  'subject' => $mail[0]['subject'],
                  'message' => $mail[0]['body'],
                ),

                'account'  => $account,
                'password' => $password,
              )
              , 'dmitriylomot@up-lvl.ru');
          }
          else {
            drupal_mail('system', __FUNCTION__, $value['name'], $language,
              array(
                'context'  => array(
                  'subject' => $mail[0]['subject'],
                  'message' => $mail[0]['body'],
                ),
                'account'  => $account,
                'password' => $password,
              ));
          }


//          ditoolsi_training_send_mail_to_pupil($account, $form_state['storage']['course_id']);
          db_insert('ditoolsi_training_pupils_list')
            ->fields(array(
              'course_id'      => $form_state['storage']['course_id'],
              'user_id'        => $account->uid,
              'current_lesson' => $value['lesson'],
              'created'        => time(),
              'status'         => DITOOLSI_TRAINING_PUPIL_ACTIVE
            ))
            ->execute();
          drupal_set_message(t('Pupil @user successfully added to your course',
            array('@user' => $account->mail)));
          mgc_start_learn2_from_teacher($course, $account);
        }
      }
      else {
        drupal_set_message(t('Wrong e-mail format @email',
          array('@email' => $value['name'])), 'error');
      }
    }
    else {
      drupal_set_message(t('Field e-mail can not be empty'), 'error');
    }
  }

  return $form;
}

/**
 * "Add more" button submit callback.
 */
function ditoolsi_training_form_add_more_submit($form, &$form_state) {
  $form_state['storage']['name_count']++;
  $form_state['rebuild'] = TRUE;
}

/**
 * "Add more" button ajax callback.
 */
function ditoolsi_training_form_update_ajax($form, $form_state) {


  return $form['names'];
}

function ditoolsi_training_add_pupils_excel_form($form, &$form_state, $course) {
  if (!function_exists('phpexcel_import')) {
    form_load_include($form_state, 'inc', 'phpexcel', 'phpexcel');
  }
  if (empty($form_state['storage']['course_id'])) {
    $form_state['storage']['course_id'] = $course->nid;
  }
  $form['#id']        = 'add-pupils-excel';
  $csv_file           = l('csv', variable_get('csv_file', ''), array(
    'html'       => TRUE,
    'absolute'   => TRUE,
    'attributes' => array(
      'class' => array(
        'excel-link-csv',
      ),
    ),
  ));
  $xls_file           = l('xls', variable_get('xls_file', ''), array(
    'html'       => TRUE,
    'absolute'   => TRUE,
    'attributes' => array(
      'class' => array(
        'excel-link-xls',
      ),
    ),
  ));
  $excel_link         = t('(Download example file !csv and !xls)', array(
    '!csv' => $csv_file,
    '!xls' => $xls_file
  ));
  $form['excel_link'] = array(
    '#markup' => '<div class="excel-link">' . $excel_link . '</div>',
  );
  $form['files']      = array(
    '#type'  => 'item',
    '#title' => t('Add pupil massivly'),
  );

  $form['file_fid']    = array(
    '#type'              => 'managed_file',
    '#size'              => 5000,
    '#upload_location'   => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => array('xls xlsm xlsb csv'),
      'file_validate_size'       => array(103809024),
    ),
    '#field_prefix'      => '<div class="ditoolsi-file-field">',
    '#field_suffix'      => '</div>',
  );
  $description         = t('Max file size: @size Mb. Allowed file types: .csv / .xls',
    array('@size' => DITOOLSI_TRAINING_EXCEL_SIZE));
  $form['description'] = array(
    '#markup' => '<div class="file-description">' . $description . '</div>',
  );

  $form['submit'] = array(
    '#attributes' => array(
      'class' => array(
        'load-pupils'
      ),
    ),
    '#type'       => 'submit',
    '#value'      => t('Process table'),
    '#submit'     => array('ditoolsi_training_add_pupils_excel_submit'),
    '#ajax'       => array(
      'callback' => 'ditoolsi_training_add_pupils_excel_ajax',
      'wrapper'  => 'add-pupils-excel',
      'effect'   => 'fade',
      'progress' => array(
        'type' => 'bar'
      ),
      'event'    => 'click',
    ),
  );

  return $form;
}

function ditoolsi_training_add_pupils_excel_submit($form, &$form_state) {
  $course_id = $form_state['storage']['course_id'];
  $file_fid  = $form_state['values']['file_fid'];

  if (empty($file_fid)) {
    $form_state['rebuild'] = TRUE;

    return;
  }

  $data   = ditoolsi_excel_parse_data_file($file_fid);
  $status = ditoolsi_excel_update_data_file($data, $course_id);

  if ($status !== TRUE) {
    $form_state['rebuild'] = TRUE;

    return;
  }

  file_delete(file_load($file_fid));
}

function ditoolsi_training_add_pupils_excel_ajax($form, $form_state) {
  return $form;
}

function ditoolsi_excel_parse_data_file($file_fid) {
  $file           = file_load($file_fid);
  $formatted_data = array();

  $path_parts     = pathinfo($file->uri);
  $file_extension = $path_parts['extension'];

  if ($file_extension == 'csv') {
    $filename = $file->uri;
    $header   = NULL;
    $num_row  = 1;
    if (($handle = fopen($filename, "r")) !== FALSE) {
      while (($row = fgetcsv($handle, 1000)) !== FALSE) {
        if (!$header) {
          $header = $row;
          if ($header[0] !== DITOOLSI_RECRUITMENT_EMAIL) {
            return array(
              'error'     => DITOOLSI_RECRUITMENT_EMAIL_INVALID,
              'sheet_key' => 'none',
              'row_key'   => '1',
              'column'    => DITOOLSI_RECRUITMENT_EMAIL,
            );
          }
        }
        else {
          if (!valid_email_address($row[0])) {
            return array(
              'error'     => DITOOLSI_RECRUITMENT_EMAIL_INVALID,
              'sheet_key' => 'none',
              'row_key'   => $num_row,
              'column'    => DITOOLSI_RECRUITMENT_EMAIL,
            );
          }
          $formatted_data[] = array_combine($header, $row);
        }
        $num_row++;
      }
      fclose($handle);
    }
  }
  else {
    $parsed_data = phpexcel_import($file->uri, TRUE, TRUE);
    if (is_array($parsed_data)) {
      foreach ($parsed_data as $sheet_key => $rows) {
        foreach ($rows as $row_key => $row) {
          if (empty($row[DITOOLSI_RECRUITMENT_EMAIL])
            || !valid_email_address($row[DITOOLSI_RECRUITMENT_EMAIL])
          ) {
            return array(
              'error'     => DITOOLSI_RECRUITMENT_EMAIL_INVALID,
              'sheet_key' => $sheet_key,
              'row_key'   => ($row_key + 2),
              'column'    => DITOOLSI_RECRUITMENT_EMAIL,
            );
          }
          $formatted_data[] = $row;
        }
      }
    }
  }

  return $formatted_data;
}

function ditoolsi_excel_update_data_file($data, $course_id) {
  global $language;

  $node = node_load($course_id);
  if (!empty($data['error']) && $data['error'] == DITOOLSI_RECRUITMENT_EMAIL_INVALID) {
    drupal_set_message(t('Your Excel file is invalid. Sheet: @sheet. Row: @row. Column: @col',
      array(
        '@sheet' => $data['sheet_key'],
        '@row'   => $data['row_key'],
        '@col'   => $data['column'],
      )), 'error');

    return FALSE;
  }
  elseif ($data == PHPEXCEL_ERROR_FILE_NOT_READABLE) {
    drupal_set_message(t('An error occurred. Please reload page and try newly'),
      'error');

    return FALSE;
  }
  elseif (is_array($data)) {
    foreach ($data as $row) {
      $email   = $row[DITOOLSI_RECRUITMENT_EMAIL];
      $account = user_load_by_mail($email);
      if (!empty($account)) {
        $query = db_select('ditoolsi_training_pupils_list', 'tpl');
        $query->fields('tpl', array('id'));
        $query->condition('tpl.user_id', $account->uid);
        $query->condition('tpl.course_id', $course_id);
        $results = $query->execute()->fetchAll();

        if (empty($results)) {
          $query = db_select('field_data_field_lesson_course', 'lr');
          $query->innerJoin('field_data_field_lesson_number', 'lw',
            'lw.entity_id = lr.entity_id');
          $query->fields('lr', array('entity_id'));
          $query->condition('lr.field_lesson_course_target_id', $course_id);
          $query->orderBy('lw.field_lesson_number_value', 'asc');
          $query->range(0, 1);
          $res = $query->execute()->fetchAll();

          $query = new EntityFieldQuery();
          $query
            ->entityCondition('entity_type', 'node')
            ->entityCondition('bundle', 'lesson')
            ->fieldCondition('field_lesson_course', 'target_id', $course_id)
            ->fieldCondition('field_lesson_number', 'value',
              DITOOLSI_TRAINING_FIRST_LESSON);

          $result = $query->execute();

          if (isset($result['node'])) {
            $lid    = key($result['node']);
            $lesson = node_load($lid);
            dt_training_update_start_time_progress($lesson, $account);
          }

          db_insert('ditoolsi_training_pupils_list')
            ->fields(array(
              'course_id'      => $course_id,
              'user_id'        => $account->uid,
              'current_lesson' => DITOOLSI_TRAINING_FIRST_LESSON,
              'created'        => time(),
              'status'         => DITOOLSI_TRAINING_PUPIL_ACTIVE
            ))
            ->execute();

          drupal_set_message(t('Pupil @mail successfully added to course',
            array('@mail' => $email)));
          $mail               = ditoolsi_training_get_mail_activation($node);
          $mail[1]['body']    = token_replace($mail[1]['body'], array(
            'node' => $node,
            'user' => $account,
          ));
          $mail[1]['subject'] = token_replace($mail[1]['subject'], array(
            'node' => $node,
            'user' => $account,
          ));
          drupal_mail('system', __FUNCTION__, $account->mail, $language, array(
            'context' => array(
              'subject' => $mail[1]['subject'],
              'message' => $mail[1]['body'],
            ),
          ));
        }
        else {
          drupal_set_message(t('Pupil @mail is already on this course',
            array('@mail' => $email)), 'error');
        }
      }
      else {
        $roles   = user_roles();
        $fields  = array(
          'name'   => $email,
          'mail'   => $email,
          'pass'   => DITOOLSI_DEFAULT_PASS_USER,
          'status' => 1,
          'init'   => $email,
          'access' => time(),
          'roles'  => array(
            DITOOLSI_PUPIL_RID => $roles[DITOOLSI_PUPIL_RID]
          ),
        );
        $account = user_save('', $fields);
        $account = ditoolsi_profile_account($account, FALSE);

        $query = db_select('field_data_field_lesson_course', 'lr');
        $query->innerJoin('field_data_field_lesson_number', 'lw',
          'lw.entity_id = lr.entity_id');
        $query->fields('lr', array('entity_id'));
        $query->condition('lr.field_lesson_course_target_id', $course_id);
        $query->orderBy('lw.field_lesson_number_value', 'asc');
        $query->range(0, 1);
        $res = $query->execute()->fetchAll();

        $query = new EntityFieldQuery();
        $query
          ->entityCondition('entity_type', 'node')
          ->entityCondition('bundle', 'lesson')
          ->fieldCondition('field_lesson_course', 'target_id', $course_id)
          ->fieldCondition('field_lesson_number', 'value',
            DITOOLSI_TRAINING_FIRST_LESSON);

        $result = $query->execute();

        if (isset($result['node'])) {
          $lid    = key($result['node']);
          $lesson = node_load($lid);
          dt_training_update_start_time_progress($lesson, $account);
        }

        $mail               = ditoolsi_training_get_mail_activation($node);
        $mail[0]['subject'] = token_replace($mail[0]['subject'], array(
          'node' => $node,
          'user' => $account,
        ));
        $mail[0]['body']    = token_replace($mail[0]['body'], array(
          'node' => $node,
          'user' => $account,
        ));
        drupal_mail('system', __FUNCTION__, $email, $language, array(
          'context' => array(
            'subject' => $mail[0]['subject'],
            'message' => $mail[0]['body'],
          ),
        ));

        db_insert('ditoolsi_training_pupils_list')
          ->fields(array(
            'course_id'      => $course_id,
            'user_id'        => $account->uid,
            'current_lesson' => DITOOLSI_TRAINING_FIRST_LESSON,
            'created'        => time(),
            'status'         => DITOOLSI_TRAINING_PUPIL_ACTIVE
          ))
          ->execute();

        drupal_set_message(t('Pupil @mail successfully added to course',
          array('@mail' => $email)));

        // $from = variable_get('site_mail', '');
        // $to_admin[1] = 'addminoz@gmail.com';
        // $to_admin[2] = 'evgenytop@gmail.com';

        // $course = node_load($course_id);
        // $title = t('Новый ученик на сайте');
        // $mess = 'На курсе ' . $course->title . ' зарегистрирован новый ученик. Для модерации пройдите по ссылке: ' . $base_url . '/user/' . $account->uid . '/edit';

        // $params = array(
        //   'subject' => $title,
        //   'body' => $mess,
        // );

        // foreach ($to_admin as $value) {
        //   drupal_mail('system', 'mail', $value, language_default(),  array(
        //     'context' => array(
        //       'subject' => $title,
        //       'message' => $mess,
        //     )
        //   ), $from);
        // }

        // $str_body = _user_mail_text('register_no_approval_required_body', NULL, array(), FALSE);
        // $symbol_body = $data_excel[1];
        // $mailbody = str_replace('[user:mail]', "$symbol_body", $str_body);

        // $str = _user_mail_text('register_pending_approval_subject', NULL, array(), FALSE);
        // $symbol = $data_excel[1];
        // $mailsubject = str_replace('[user:name]', "$symbol", $str);

        // drupal_mail('system', 'mail', $data_excel[1], language_default(),  array(
        //   'context' => array(
        //     'subject' => $mailsubject,
        //     'message' => $mailbody,
        //   )
        // ), $from);
      }
    }
    drupal_set_message(t('Loading pupils complete'));
  }

  return TRUE;
}

/**
 * Create a form for review all requests for cheking lessons from all courses.
 */
function ditoolsi_training_requests_review_form($form, &$form_state) {
  $default_value = 0;
  $course_id     = 0;
  $options       = array();

  if (isset($_REQUEST["course"])) {
    $course_id = intval($_REQUEST["course"]);
  }

  $form['#id']   = 'requests-review';
  $form['title'] = array(
    '#type'   => 'item',
    '#title'  => t('Working with requests review on the parameters'),
    '#prefix' => '<div class="title-working-param">',
    '#suffix' => '</div>',
  );

  $form['user_mail']['user_mail_title'] = array(
    '#type'  => 'item',
    '#title' => t('Search on e-mail:'),
  );

  $form['user_mail']['mail'] = array(
    '#type'              => 'textfield',
    '#title'             => t('User e-mail'),
    '#size'              => 60,
    '#maxlength'         => 128,
    '#prefix'            => "<div class='process-ditools-labels'>",
    '#suffix'            => "</div>",
    '#autocomplete_path' => 'courses-pupils/autocomplete',
    '#ajax'              => array(
      'event'    => 'blur',
      'wrapper'  => 'requests-review',
      'callback' => 'ditoolsi_training_requests_review_ajax_callback',
    ),
  );

  $course_query = ditoolsi_training_get_cources_query();
  $result       = $course_query->execute()->fetchAllKeyed(0, 1);
  $nids         = array_keys($result);
  $courses      = node_load_multiple($nids);
  $options[0]   = t('All cources');

  foreach ($courses as $course) {
    $options[$course->nid] = $course->title;
  }

  if (in_array($course_id, $nids)) {
    $default_value = $course_id;
  }

  $form['courses']['course_title'] = array(
    '#type'  => 'item',
    '#title' => t('Search on course:'),
  );

  $form['courses']['course'] = array(
    '#type'          => 'select',
    '#options'       => $options,
    '#default_value' => $default_value,
    '#ajax'          => array(
      'event'    => 'change',
      'wrapper'  => 'requests-review',
      'callback' => 'ditoolsi_training_requests_review_ajax_callback',
    ),
  );

  $query = ditoolsi_training_requests_review_query();
  $stop  = 'Stop';
  if (isset($form_state['values']['mail']) && $form_state['values']['mail'] !== '') {
    $query->condition('u.mail',
      '%' . db_like($form_state['values']['mail']) . '%', 'LIKE');
  }

  if (!empty($default_value)) {
    $query->condition('node_course.nid', $default_value);
  }
  elseif (isset($form_state['values']['course']) && $form_state['values']['course'] != 0) {
    $query->condition('node_course.nid', $form_state['values']['course']);
  }
  else {
    $query->condition('node_course.nid', $nids, 'IN');
  }

  $parts = parse_url(request_uri());

  if (isset($parts['query'])) {
    parse_str($parts['query'], $query_str);
  }
  else {
    $query_str = array();
  }

  if (!empty($form_state['input']['finished'])) {
    $query->join('field_data_field_lesson_final', 'flf',
      'flf.entity_id = lesson_id.field_progress_lesson_id_target_id');
    $query->condition('flf.field_lesson_final_value', 1);
    $query_str['finished'] = NULL;
  }
  elseif (!empty($form_state['input']['not_finished'])) {
    $query->join('field_data_field_lesson_final', 'flf',
      'flf.entity_id = lesson_id.field_progress_lesson_id_target_id');
    $query->condition('flf.field_lesson_final_value', 1);
    $query_str['not-finished'] = NULL;
  }

  if (!empty($form_state['values']['course'])) {
    $query_str['course'] = $form_state['values']['course'];
  }

  $form['finished'] = array(
    '#type'  => 'hidden',
    '#value' => (int) array_key_exists('finished', $query_str),
  );

  $form['not_finished'] = array(
    '#type'  => 'hidden',
    '#value' => (int) array_key_exists('not-finished', $query_str),
  );

  $links = array();

  if (isset($query_str['finished']) || isset($query_str['not-finished'])) {
    $query_str2 = $query_str;

    if (array_key_exists('finished', $query_str2)) {
      unset($query_str2['finished']);
    }

    if (array_key_exists('not-finished', $query_str2)) {
      unset($query_str2['not-finished']);
    }

    array_unshift($links,
      l('Показать все заявки', 'courses/requests-review', array(
        'query' => $query_str2,
      )));
  }

  if (array_key_exists('finished', $query_str)) {
    unset($query_str['finished']);
    $query->join('field_data_field_lesson_final', 'flf',
      'flf.entity_id = lesson_id.field_progress_lesson_id_target_id');
    $query->condition('flf.field_lesson_final_value', 1);
  }
  elseif (array_key_exists('not-finished', $query_str)) {
    unset($query_str['not-finished']);
    $query->join('field_data_field_lesson_final', 'flf',
      'flf.entity_id = lesson_id.field_progress_lesson_id_target_id');
    $query->condition('flf.field_lesson_final_value', 0);
  }
  $results  = $query->extend('PagerDefault')
    ->limit(MGC_REQUESTS_FOR_REVIEW_PER_PAGE)
    ->execute()
    ->fetchCol();
  $requests = node_load_multiple($results);

  $par1 = mgc_get_requests_for_review_count();
  $par2 = mgc_get_requests_for_review_w_empty_statuses_count();
  $stop = 'Stop';
  $links += array(
    'not-finished' => l('Скрыть заявки с последнего урока',
      'courses/requests-review', array(
        'query' => $query_str + array('not-finished' => NULL),
      )),
    'finished'     => l("Показать заявки с последнего урока / {$par1} / {$par2}",
      'courses/requests-review', array(
        'query' => $query_str + array('finished' => NULL),
      )),
  );

  foreach ($links as $key => $link) {
    if (isset($_GET[$key])) {
      unset($links[$key]);
    }
  }

  $form['finish_link'] = array(
    '#type'   => 'item',
    '#markup' => implode(' | ', $links),
  );

  $header = array(
    'number'        => t('№'),
    'user_name'     => t('Name'),
    'user_mail'     => t('E-mail'),
    'data_activity' => t('Date of send to review'),
    'lesson_pupil'  => t('№ lesson'),
    'pupil_log'     => t('Link for review'),
    'status'        => t('Status'),
    'time_check'    => 'Время на проверку',
  );

  //$table_num = 1;
  $start_element = 1;
  if (isset($_REQUEST['page'])) {
    $start_element = MGC_REQUESTS_FOR_REVIEW_PER_PAGE * $_REQUEST['page'] + 1;
  }
  $table_num = $start_element;
  $rows      = array();


  foreach ($requests as $request) {
    $r_wrapper     = entity_metadata_wrapper('node', $request);
    $account       = $r_wrapper->author->value();
    $name          = ditoolsi_profile_name($account);
    $mail          = $account->mail;
    $time          = $request->changed;
    $date          = format_date($time);
    $lesson_id     = $request->field_progress_lesson_id[LANGUAGE_NONE][0]['target_id'];
    $lesson        = node_load($lesson_id);
    $lesson_number = $lesson->field_lesson_number[LANGUAGE_NONE][0]['value'];
    $course_id     = $lesson->field_lesson_course[LANGUAGE_NONE][0]['target_id'];
    $time_check    = isset($lesson->field_lesson_check_time[LANGUAGE_NONE][0]['value']) ? $lesson->field_lesson_check_time[LANGUAGE_NONE][0]['value'] : FALSE;
    $time_sent     = isset($request->field_progress_time_sent[LANGUAGE_NONE][0]['value']) ? $request->field_progress_time_sent[LANGUAGE_NONE][0]['value'] : FALSE;
    $course        = node_load($course_id);

    if (isset($course->field_course_lesson_statuses[LANGUAGE_NONE]) && !empty($lesson->field_lesson_final[LANGUAGE_NONE][0]['value'])) {
      $options = array();
      $colors  = array();

      foreach ($course->field_course_lesson_statuses[LANGUAGE_NONE] as $item) {
        $fc_item = field_collection_item_load($item['value']);

        if (isset($fc_item->field_request_custom_status[LANGUAGE_NONE][0]['value'])) {
          $status_value  = $fc_item->field_request_custom_status[LANGUAGE_NONE][0]['value'];
          $key           = md5($status_value);
          $options[$key] = $status_value;

          if (isset($fc_item->field_request_custom_status_col[LANGUAGE_NONE][0]['jquery_colorpicker'])) {
            $colors[$key] = $fc_item->field_request_custom_status_col[LANGUAGE_NONE][0]['jquery_colorpicker'];
          }
          else {
            $colors[$key] = '';
          }

        }
      }

      array_unshift($options, 'Выберите статус');
      array_unshift($colors, '1c1c1c');

      $select = array(
        '#type'       => 'select',
        '#options'    => $options,
        '#attributes' => array(
          'class'       => array('custom-status'),
          'data-nid'    => $request->nid,
          'data-colors' => json_encode($colors),
        ),
      );

      if (isset($request->field_progress_custom_status[LANGUAGE_NONE][0]['value'])) {
        $select['#value'] = $request->field_progress_custom_status[LANGUAGE_NONE][0]['value'];
      }
    }
    else {
      $select = array();
    }
    $stop   = $table_num;
    $rows[] = array(
      'number'        => $table_num,
      'user_name'     => $name,
      'user_mail'     => l($mail, 'user/' . $account->uid, array(
        'attributes' => array(
          'target' => '_blank',
        ),
      )),
      'data_activity' => $date,
      'lesson_pupil'  => $lesson_number,
      'pupil_log'     => l(t('Check task'),
        "course/{$course_id}/{$lesson_id}/{$request->nid}/review"),
      'status'        => render($select),
      'time_check'    => ($time_check && $time_sent ? '<div class="timer">' . (($time_check * 24 * 3600 + $time_sent) - REQUEST_TIME) . '</div>' : '-'),
    );

    $table_num++;
  }

  $table_requests = theme('table', array(
      'header' => $header,
      'rows'   => $rows,

      'empty' => t('No requests found'),
    )
  );
  $table_requests .= theme('pager');

  $form['table'] = array(
    '#type'   => 'item',
    '#title'  => t('From'),
    '#markup' => $table_requests,
  );

  // $form['#attached'] = array(
  //   'css' => array(
  //     drupal_get_path('theme', 'bootstrap_subtheme') . '/css/list-pupils.css',
  //   ),
  // );

  libraries_load('alertify.js');

  return $form;
}

function ditoolsi_training_requests_review_ajax_callback($form, $form_state) {
  $form_state['rebuild'] = TRUE;

  return $form;
}

////////////////////////////////////////////////////////////////////////////////
function ditoolsi_training_list_pupils_form($form, &$form_state, $course) {
  if (!function_exists('phpexcel_import')) {
    form_load_include($form_state, 'inc', 'phpexcel', 'phpexcel');
  }

  $form['#id'] = 'list-pupils';

  if (!isset($form_state['storage']['course'])) {
    $form_state['storage']['course'] = $course;
  }

  $form['course_title'] = array(
    '#type'   => 'item',
    '#title'  => $course->title,
    '#prefix' => '<div class="course-title">',
    '#suffix' => '</div>',
  );
  $form['title']        = array(
    '#type'   => 'item',
    '#title'  => t('Working with a list pupils on the parameters'),
    '#prefix' => '<div class="title-working-param">',
    '#suffix' => '</div>',
  );
  $form['user_mail']    = array(
    '#type'      => 'textfield',
    '#title'     => t('User e-mail'),
    '#size'      => 60,
    '#maxlength' => 128,
    '#prefix'    => "<div class='process-ditools-labels'>",
    '#suffix'    => "</div>",
  );

  $form['date_search'] = array(
    '#tree'   => TRUE,
    '#prefix' => '<div class="date-search">',
    '#suffix' => '</div>',
  );

  $form['date_search']['pupil_status'] = array(
    '#type'          => 'select',
    '#title'         => t('Search on activity:'),
    '#options'       => array(
      'any' => t('All'),
      1     => t('Active', array(), array('context' => 'Act')),
      2     => t('Blocked', array(), array('context' => 'Block'))
    ),
    '#default_value' => 0,
  );

  $form['date_search']['date_from'] = array(
    '#type'   => 'textfield',
    '#prefix' => '<div class="selecting-date day">',
    '#suffix' => '<span class="selected"></span><div class="date-picker ditoolsi-calendar"></div></div>',
  );

  $form['date_search']['date_till'] = array(
    '#type'   => 'textfield',
    '#prefix' => '<div class="selecting-date day">',
    '#suffix' => '<span class="selected"></span><div class="date-picker ditoolsi-calendar"></div></div>',
  );
  $form['search_by_lessons']        = array(
    '#tree'   => TRUE,
    '#prefix' => '<div id="by-lesson-search">',
    '#suffix' => '</div>',
  );

  $form['search_by_lessons']['by_lessons_title'] = array(
    '#type'  => 'item',
    '#title' => t('Search on lessons:'),
  );

  $query   = ditoolsi_training_list_pupils_lesson_query($course);
  $result  = $query->execute()->fetchAll();
  $options = array();
  foreach ($result as $lesson) {
    $options[$lesson->field_lesson_number_value] = $lesson->field_lesson_number_value;
  }

  $last_lesson = end($options);
  $last_lesson = key($options);

  $first_lesson = reset($options);
  $first_lesson = key($options);

  $form['search_by_lessons']['lessons_from'] = array(
    '#type'          => 'select',
    '#options'       => $options,
    '#default_value' => $first_lesson,
  );

  $form['search_by_lessons']['lessons_till'] = array(
    '#type'          => 'select',
    '#options'       => $options,
    '#default_value' => $last_lesson,
  );

  $form['search_by_group'] = array(
    '#tree'   => TRUE,
    '#prefix' => '<div class="group-search">',
    '#suffix' => '</div>',
  );

  $groups = get_group_list_by_teacher();
  $groups['all'] = 'Все группы';
  $form['search_by_group']['user_group'] = array(
    '#type'          => 'select',
    '#title'         => 'Поиск по группе',
    '#options'       => $groups,
    '#default_value' => 'all',
  );

  $form['submit'] = array(
    '#type'       => 'submit',
    '#value'      => t('Search'),
    '#attributes' => array('class' => array('search-button')),
    '#ajax'       => array(
      'callback' => 'ditoolsi_training_list_pupils_ajax_callback',
      'wrapper'  => 'list-pupils',
    ),
    '#submit'     => array('ditoolsi_training_list_pupils_submit'),
  );


  $query = ditoolsi_training_list_pupils_query($course)
    ->extend('PagerDefault')
    ->limit(30);

  if (isset($form_state['storage']['email']) && $form_state['storage']['email'] !== '') {
    $query->condition('u.mail',
      '%' . db_like($form_state['storage']['email']) . '%', 'LIKE');
  }

  if (isset($form_state['storage']['status']) && $form_state['storage']['status'] !== 'any') {
    $query->condition('pupils_list.status', $form_state['storage']['status']);
  }

  if (isset($form_state['storage']['group']) && $form_state['storage']['group'] !== 'all' ) {
    if($form_state['storage']['group'] !== '0') {
      $query->condition('user_group.field_user_group_tid', $form_state['storage']['group']);
    }
    else {
      $query->condition('user_group.field_user_group_tid', NULL, 'IS NULL');
    }
  }

  if (isset($form_state['storage']['date_from']) && $form_state['storage']['date_from'] !== '' && isset($form_state['storage']['date_till']) && $form_state['storage']['date_till'] !== '') {
    $query->condition('u.access',
      strtotime($form_state['storage']['date_from']), '>=');
    $query->condition('u.access',
      strtotime($form_state['storage']['date_till']), '<=');
  }

  if (isset($form_state['storage']['lessons_from']) && isset($form_state['storage']['lessons_till'])) {
    // $query->condition('pupils_list.current_lesson', $form_state['storage']['lessons_from'], '>=');
    // $query->condition('pupils_list.current_lesson', $form_state['storage']['lessons_till'], '<=');

    $query->condition(
      db_or()
        ->isNull('pupils_list.current_lesson')
        ->condition(
          db_and()
            ->condition('pupils_list.current_lesson',
              $form_state['storage']['lessons_from'], '>=')
            ->condition('pupils_list.current_lesson',
              $form_state['storage']['lessons_till'], '<=')
        )
    );
  }

  $results = $query->execute()->fetchAll();
  $header  = array(
    'number'        => t('№'),
    'user_name'     => t('Name'),
    'user_mail'     => t('E-mail'),
    'data_activity' => t('Date of activity'),
    'pupil_status'  => t('Status'),
    'pupil_group'  => t('Group'),
    'lesson_pupil'  => t('№ lesson'),
    'pupil_log'     => t('History')
  );


  $table_num    = 1;
  $table_users  = array();
  $first_lesson = ditoolsi_training_get_first_lesson($course);
  $page         = isset($_GET['page']) ? $_GET['page'] : 0;
  foreach ($results as $u_key => $u_value) {
    $name = $u_value->field_first_name_value || $u_value->field_middle_name_value || $u_value->field_last_name_value;
    $name = $name ? implode(' ', array(
      $u_value->field_first_name_value,
      $u_value->field_middle_name_value,
      $u_value->field_last_name_value
    )) : '';

    if(empty($u_value->field_user_group_tid)){
      $group_name = '--';
    }
    else {
      $group_tid = $u_value->field_user_group_tid;
      $group = taxonomy_term_load($group_tid);
      $group_name = $group->name;
    }

    $form['lessons']            = array(
      '#type'       => 'select',
      '#options'    => $options,
      '#value'      => $u_value->current_lesson,
      '#attributes' => array(
        'class'   => array("lesson-select-user select-pupil-lesson-switch"),
        'user_id' => array($u_value->user_id)
      )
    );
    $form['user_passed_lesson'] = getUserLessonList($course->nid, $result,
      $u_value->user_id);

    $table_users[$u_value->user_id] = array(
      'number'        => ($page * 30) + $table_num,
      'user_name'     => $name,
      'user_mail'     => $u_value->mail,
      'data_activity' => date('d M Y', $u_value->access),
      'pupil_status'  => $u_value->status == 1 ? t('Active', array(),
        array('context' => 'Act')) : t('Blocked', array(),
        array('context' => 'Block')),
      'pupil_group'   => $group_name,
      'lesson_pupil'  => drupal_render($form['lessons']),
      'pupil_log'     => drupal_render($form['user_passed_lesson']),
    );

    $table_num++;
  }

  $form['table'] = array(
    '#type'    => 'tableselect',
    '#header'  => $header,
    '#options' => $table_users,
    '#empty'   => t('No users found'),
    '#suffix'  => theme('pager'),
  );

  $form['activate_pupils'] = array(
    '#type'   => 'submit',
    '#value'  => t('Activate'),
    '#ajax'   => array(
      'callback' => 'ditoolsi_training_list_pupils_activate_ajax_callback',
      'wrapper'  => 'list-pupils',
    ),
    '#submit' => array('ditoolsi_training_list_pupils_activate_submit'),
  );

  $form['block_pupils'] = array(
    '#type'   => 'submit',
    '#value'  => t('Blockade'),
    '#ajax'   => array(
      'callback' => 'ditoolsi_training_list_pupils_block_ajax_callback',
      'wrapper'  => 'list-pupils',
    ),
    '#submit' => array('ditoolsi_training_list_pupils_block_submit'),
  );

  $form['delete_pupils'] = array(
    '#type'   => 'submit',
    '#value'  => t('Delete'),
    '#ajax'   => array(
      'callback' => 'ditoolsi_training_list_pupils_delete_ajax_callback',
      'wrapper'  => 'list-pupils',
    ),
    '#submit' => array('ditoolsi_training_list_pupils_delete_submit'),
  );

  $form['upload_list']        = array(
    '#type'   => 'submit',
    '#value'  => t('Upload list'),
    '#ajax'   => array(
      'callback' => 'ditoolsi_training_list_pupils_upload_ajax_callback',
      'wrapper'  => 'list-pupils',
    ),
    '#submit' => array('ditoolsi_training_list_pupils_upload_submit'),
  );
  $form['list-pupils-status'] = array(
    '#type'   => 'container',
    '#suffix' => '<div class="list-pupils-status"></div>'
  );

  $form['add_to_group'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('add-to-group-wrap')
    ),
  );

  if(!mgc_user_has_curator_role()){
    $form['add_to_group']['list_group'] = array(
      '#type' => 'select',
      '#title' => 'Добавить в группу:',
      '#options' => get_group_list_by_teacher(),
      '#default_value' => 0,
    );

    $form['add_to_group']['submit'] = array(
      '#type'   => 'submit',
      '#value'  => 'Добавить',
      '#ajax'   => array(
        'callback' => 'ditoolsi_training_add_user_to_group_ajax_callback',
        'wrapper'  => 'list-pupils',
      ),
      '#submit' => array('ditoolsi_training_add_user_to_group_submit'),
    );
  }


  $form['#attached']          = array(
    'css' => array(
      drupal_get_path('theme', 'bootstrap_subtheme') . '/css/list-pupils.css',
    ),
  );

  return $form;
}

function ditoolsi_training_list_pupils_submit($form, &$form_state) {
  $form_state['storage']['email']        = $form_state['values']['user_mail'];
  $form_state['storage']['status']       = $form_state['values']['date_search']['pupil_status'];
  $form_state['storage']['date_from']    = $form_state['values']['date_search']['date_from'];
  $form_state['storage']['date_till']    = $form_state['values']['date_search']['date_till'];
  $form_state['storage']['lessons_from'] = $form_state['values']['search_by_lessons']['lessons_from'];
  $form_state['storage']['lessons_till'] = $form_state['values']['search_by_lessons']['lessons_till'];
  $form_state['storage']['group']        = $form_state['values']['search_by_group']['user_group'];
  $form_state['rebuild']                 = TRUE;
}

function ditoolsi_training_list_pupils_ajax_callback($form, $form_state) {
  return $form;
}

function get_group_list_by_teacher(){

  global $user;
  $groups_term = get_groups_by_teacher($user);

  $groups = array(0 => 'Без группы');
  foreach ($groups_term as $group) {
    $groups[$group->tid] = $group->name;
  }

  return $groups;

}

function ditoolsi_training_add_user_to_group_ajax_callback($form, $form_state) {
  return $form;
}

function ditoolsi_training_add_user_to_group_submit($form, &$form_state) {
  $user_ids = array_filter($form_state['values']['table']);
  $user_ids = array_values($user_ids);
  $tid = $form_state['values']['list_group'];
  if (!empty($user_ids)) {
    foreach ($user_ids as $uid) {
      $user = user_load($uid);
      if($tid) {
        $user->field_user_group['und'][0]['tid'] = $tid;
      }
      else {
        $user->field_user_group = NULL;
      }
      user_save($user);
    }

    $form_state['rebuild'] = TRUE;
    return $form;

  }
}

function ditoolsi_training_list_pupils_activate_submit($form, &$form_state) {
  $course   = $form_state['storage']['course'];
  $user_ids = array_unique($form_state['values']['table']);
  if (!empty($user_ids)) {
    $query = db_select('ditoolsi_training_pupils_list', 'pl');
    $query
      ->fields('pl')
      ->condition('pl.course_id', $course->nid)
      ->condition('pl.user_id', $user_ids, 'IN');

    $result = $query->execute()->fetchAll();

    foreach ($result as $item) {
      $lesson = dt_training_load_lesson_by_number($course->nid,
        $item->current_lesson);

      if ($lesson) {
        dt_training_update_start_time_progress($lesson, $item->user_id);
      }
    }

    db_update('ditoolsi_training_pupils_list')
      ->fields(array('status' => DITOOLSI_TRAINING_PUPIL_ACTIVE))
      ->condition('course_id', $course->nid)
      ->condition('user_id', $user_ids, 'IN')
      ->execute();

    $form_state['rebuild'] = TRUE;

    return;
  }
}

function ditoolsi_training_list_pupils_activate_ajax_callback(
  $form,
  $form_state
) {
  $sum = array_sum($form_state['values']['table']);

  if (empty($sum)) {
    drupal_set_message(t('Select users'), 'error');
  }
  else {
    drupal_set_message(t('All users activate'));
  }

  return $form;
}

function ditoolsi_training_list_pupils_block_submit($form, &$form_state) {
  $course   = $form_state['storage']['course'];
  $user_ids = array_unique($form_state['values']['table']);
  if (!empty($user_ids)) {
    db_update('ditoolsi_training_pupils_list')
      ->fields(array('status' => DITOOLSI_TRAINING_PUPIL_BLOCKED))
      ->condition('course_id', $course->nid)
      ->condition('user_id', $user_ids, 'IN')
      ->execute();

    $form_state['rebuild'] = TRUE;

    return;
  }
}

function ditoolsi_training_list_pupils_block_ajax_callback($form, $form_state) {
  $sum = array_sum($form_state['values']['table']);

  if (empty($sum)) {
    drupal_set_message(t('Select users'), 'error');
  }
  else {
    drupal_set_message(t('All users Blocked'));
  }

  return $form;
}

function ditoolsi_training_list_pupils_delete_submit($form, &$form_state) {
  $course   = $form_state['storage']['course'];
  $user_ids = array_unique($form_state['values']['table']);
  if (!empty($user_ids)) {
    //notification rule
    foreach ($user_ids as $value) {
      $notification_options            = array();
      $term                            = taxonomy_term_load(DT_TRAINING_TT_DELETE);
      $account                         = user_load($value);
      $notification_options['course']  = $course;
      $notification_options['account'] = $account;
      $notification_options['term']    = $term;

      rules_invoke_event('ditoolsi_training_pupils_notification',
        $notification_options);
    }

    db_delete('ditoolsi_training_pupils_list')
      ->condition('course_id', $course->nid)
      ->condition('user_id', $user_ids, 'IN')
      ->execute();

    $form_state['rebuild'] = TRUE;


    return;
  }
}

function ditoolsi_training_list_pupils_delete_ajax_callback(
  $form,
  $form_state
) {
  $sum = array_sum($form_state['values']['table']);

  if (empty($sum)) {
    drupal_set_message(t('Select users'), 'error');
  }
  else {
    drupal_set_message(t('Users delete'));
  }

  return $form;
}

function mgc_list_all_pupils_upload($form, &$form_state) {
  $data      = array();
  $headers   = array();
  $course    = $form_state['storage']['course'];
  $query_ids = ditoolsi_training_list_pupils_query($course);
  $new_ids   = $query_ids->execute()->fetchAll();
  $name      = '';

  foreach ($new_ids as $items) {
    if ($items->field_first_name_value || $items->field_middle_name_value || $items->field_last_name_value) {
      $name = implode(' ', array(
        $items->field_first_name_value,
        $items->field_middle_name_value,
        $items->field_last_name_value
      ));
    }
    $table_num = 1;
    $data[]    = array(
      $table_num,
      $name,
      $items->mail,
      date('d M Y', $items->access),
      $items->status == 1 ? t('Active', array(),
        array('context' => 'Act')) : t('Blocked', array(),
        array('context' => 'Block')),
      $items->current_lesson,
    );
    $table_num++;
  }
  $headers = array(
    t('№'),
    t('Name'),
    t('E-mail'),
    t('Date of activity'),
    t('Status'),
    t('№ lesson'),
  );

  // Store the file in sites/default/files
  $path = 'public://export_xls';
  file_prepare_directory($path, FILE_CREATE_DIRECTORY);
  $dir                           = file_stream_wrapper_get_instance_by_uri($path)->realpath();
  $filename                      = 'export' . microtime(TRUE) . '.xls';
  $form_state['storage']['path'] = file_create_url($path . '/' . $filename);
  $path                          = "$dir/$filename";
  // Use the .xls format
  $options = array('format' => 'xls');

  $result = phpexcel_export($headers, $data, $path, $options);
  if ($result == PHPEXCEL_SUCCESS) {
    drupal_set_message(t("We did it !"));
  }
  else {
    drupal_set_message(t("Oops ! An error occured !"), 'error');
  }
  $form_state['rebuild'] = TRUE;
}

function ditoolsi_training_list_pupils_upload_submit($form, &$form_state) {
  $data     = array();
  $headers  = array();
  $course   = $form_state['storage']['course'];
  $user_ids = array_unique($form_state['values']['table']);
  if (!empty($user_ids)) {
    $query = ditoolsi_training_list_pupils_query($course);
    $query->condition('user_id', $user_ids, 'IN');
    $results = $query->execute()->fetchAll();
    $name    = '';

    foreach ($results as $items) {
      if ($items->field_first_name_value || $items->field_middle_name_value || $items->field_last_name_value) {
        $name = implode(' ', array(
          $items->field_first_name_value,
          $items->field_middle_name_value,
          $items->field_last_name_value
        ));
      }
      $table_num = 1;
      $data[]    = array(
        $table_num,
        $name,
        $items->mail,
        date('d M Y', $items->access),
        $items->status == 1 ? t('Active', array(),
          array('context' => 'Act')) : t('Blocked', array(),
          array('context' => 'Block')),
        $items->current_lesson,
      );
      $table_num++;
    }
    $headers = array(
      t('№'),
      t('Name'),
      t('E-mail'),
      t('Date of activity'),
      t('Status'),
      t('№ lesson'),
    );

    // Store the file in sites/default/files
    $path = 'public://export_xls';
    file_prepare_directory($path, FILE_CREATE_DIRECTORY);
    $dir                           = file_stream_wrapper_get_instance_by_uri($path)->realpath();
    $filename                      = 'export' . microtime(TRUE) . '.xls';
    $form_state['storage']['path'] = file_create_url($path . '/' . $filename);
    $path                          = "$dir/$filename";
    // Use the .xls format
    $options = array('format' => 'xls');

    $result = phpexcel_export($headers, $data, $path, $options);
    if ($result == PHPEXCEL_SUCCESS) {
      drupal_set_message(t("We did it !"));
    }
    else {
      drupal_set_message(t("Oops ! An error occured !"), 'error');
    }
    $form_state['rebuild'] = TRUE;
  }
}

function ditoolsi_training_list_pupils_upload_ajax_callback(
  $form,
  $form_state
) {
  $sum = array_sum($form_state['values']['table']);

  if (empty($sum)) {
    drupal_set_message(t('Select users'), 'error');
  }
  else {
    $link = l(t('Download file'), $form_state['storage']['path'],
      array('language' => LANGUAGE_NONE));
    drupal_set_message(t('Users upload'));
  }

  return render($form) . $link;
}

function ditoolsi_training_section_form_callback($library, $section = NULL) {
  global $user;

  module_load_include('inc', 'node', 'node.pages');

  if (empty($section)) {
    $section                        = (object) array(
      'uid'      => $user->uid,
      'name'     => (isset($user->name) ? $user->name : ''),
      'type'     => 'section',
      'language' => LANGUAGE_NONE,
    );
    $wrapper                        = entity_metadata_wrapper('node', $section);
    $wrapper->field_section_library = $library->nid;
  }

  return drupal_get_form('section_node_form', $section);
}

function ditoolsi_training_list_pupils_library_form(
  $form,
  &$form_state,
  $library
) {
  $form['#id'] = 'list-pupils-library';

  if (!isset($form_state['storage']['library'])) {
    $form_state['storage']['library'] = $library;
  }

  $results = ditoolsi_training_add_pupils_library_query($library)
    ->execute()
    ->fetchAll();
  $header  = array(
    'number'    => t('№'),
    'user_name' => t('Name'),
    'access'    => t('Gained access'),
  );

  $table_num   = 1;
  $table_users = array();
  foreach ($results as $items) {
    $name                     = $items->field_first_name_value || $items->field_middle_name_value || $items->field_last_name_value;
    $name                     = $name ? implode(' ', array(
      $items->field_first_name_value,
      $items->field_middle_name_value,
      $items->field_last_name_value
    )) : $items->name;
    $table_users[$items->uid] = array(
      'number'    => $table_num,
      'user_name' => $name,
      'access'    => date('d M Y', $items->created),
    );

    $table_num++;
  }

  $form['table'] = array(
    '#type'    => 'tableselect',
    '#header'  => $header,
    '#options' => $table_users,
    '#empty'   => t('No users found in library'),
  );

  $form['delete_pupils'] = array(
    '#type'     => 'submit',
    '#value'    => t('Delete'),
    '#ajax'     => array(
      'callback' => 'ditoolsi_training_list_pupils_library_delete_ajax_callback',
      'wrapper'  => 'list-pupils-library',
    ),
    '#submit'   => array('ditoolsi_training_list_pupils_library_delete_submit'),
    '#validate' => array('ditoolsi_training_list_pupils_library_delete_validate'),
  );

  $form['title'] = array(
    '#type'  => 'item',
    '#title' => t('Add pupil in library'),
  );

  $form['mail'] = array(
    '#title'             => t('E-mail pupil'),
    '#type'              => 'textfield',
    '#autocomplete_path' => 'pupils/autocomplete',
    '#prefix'            => '<div class="process-ditools-labels">',
    '#suffix'            => '</div>',
  );

  $form['submit'] = array(
    '#type'       => 'submit',
    '#value'      => t('Add'),
    '#attributes' => array('class' => array('add-button')),
    '#ajax'       => array(
      'callback' => 'ditoolsi_training_list_pupils_library_ajax_callback',
      'wrapper'  => 'list-pupils-library',
    ),
    '#submit'     => array('ditoolsi_training_list_pupils_library_submit'),
    '#validate'   => array('ditoolsi_training_list_pupils_library_validate'),
  );

  $form['#attached'] = array(
    'css' => array(
      drupal_get_path('theme',
        'bootstrap_subtheme') . '/css/add-pupil-library.css',
    ),
  );

  return $form;
}

function ditoolsi_training_list_pupils_library_submit($form, &$form_state) {
  $library = $form_state['storage']['library'];
  $mail    = $form_state['values']['mail'];
  $account = user_load_by_mail($mail);
  if (!empty($account)) {
    $query = db_select('ditoolsi_training_library_pupils', 'library_pupils');
    $query->fields('library_pupils', array('user_id'));
    $query->condition('library_pupils.user_id', $account->uid);
    $query->condition('library_pupils.library_id', $library->nid);
    $result = $query->execute()->fetchAll();

    if (empty($result)) {
      db_insert('ditoolsi_training_library_pupils')
        ->fields(array(
          'user_id'    => $account->uid,
          'library_id' => $library->nid,
          'created'    => time()
        ))
        ->execute();

      drupal_set_message(t('Pupil successfully added on your library'));
      $form_state['values']['mail'] = '';
      $form_state['input']['mail']  = '';
    }
    else {
      drupal_set_message(t('Pupil @mail is already has access to the library',
        array('@mail' => $mail)), 'error');
    }
  }
  else {
    drupal_set_message(t('Invalid data'), 'error');
  }

  $form_state['rebuild'] = TRUE;
}

function ditoolsi_training_list_pupils_library_validate($form, &$form_state) {
  if (empty($form_state['values']['mail'])) {
    form_set_error('mail', t('Field "@name" is required', array(
      '@name' => 'E-mail',
    )));
  }
}

function ditoolsi_training_list_pupils_library_ajax_callback(
  $form,
  $form_state
) {
  return $form;
}

function ditoolsi_training_list_pupils_library_delete_submit(
  $form,
  &$form_state
) {
  $library  = $form_state['storage']['library'];
  $user_ids = array_unique($form_state['values']['table']);

  if (!empty($user_ids)) {
    db_delete('ditoolsi_training_library_pupils')
      ->condition('library_id', $library->nid)
      ->condition('user_id', $user_ids, 'IN')
      ->execute();

    $form_state['rebuild'] = TRUE;
  }
}

function ditoolsi_training_list_pupils_library_delete_ajax_callback(
  $form,
  $form_state
) {
  return $form;
}

function ditoolsi_training_list_pupils_library_delete_validate(
  $form,
  &$form_state
) {
  $sum = array_sum($form_state['values']['table']);
  if (empty($sum)) {
    form_set_error('', t('Select users'));
  }
}

function ditoolsi_training_section_node_form_submit($form, &$form_state) {
  $form_state['redirect'] = '/library/' . $form_state['values']['field_section_library'][LANGUAGE_NONE][0]['target_id'];
}

/**
 * Create a form for mass messaging.
 */
function ditoolsi_training_pupil_inform_form($form, &$form_state, $course) {

  $form['#id']       = 'inform-pupils';
  $form['user_mail'] = array(
    '#type'              => 'textfield',
    '#title'             => t('User e-mail'),
    '#size'              => 60,
    '#maxlength'         => 128,
    '#prefix'            => "<div class='process-ditools-labels'>",
    '#suffix'            => "</div>",
    '#autocomplete_path' => 'course-pupils/autocomplete/' . $course->nid,
  );

  $form['date_search'] = array(
    '#tree'   => TRUE,
    '#prefix' => '<div class="date-search">',
    '#suffix' => '</div>',
  );

  $form['date_search']['date_from'] = array(
    '#type'   => 'textfield',
    '#title'  => t('Search on activity:'),
    '#prefix' => '<div class="selecting-date day">',
    '#suffix' => '<span class="selected"></span><div class="date-picker ditoolsi-calendar"></div></div>',
  );

  $form['date_search']['date_till'] = array(
    '#type'   => 'textfield',
    '#prefix' => '<div class="selecting-date day">',
    '#suffix' => '<span class="selected"></span><div class="date-picker ditoolsi-calendar"></div></div>',
  );
  $form['search_by_lessons']        = array(
    '#tree'   => TRUE,
    '#prefix' => '<div id="by-lesson-search">',
    '#suffix' => '</div>',
  );

  $form['search_by_lessons']['by_lessons_title'] = array(
    '#type'  => 'item',
    '#title' => t('Search on lessons:'),
  );
  $result                                        = ditoolsi_training_inform_pupils_lesson_query($course)
    ->execute()
    ->fetchAll();
  $options                                       = array();
  foreach ($result as $lesson) {
    $options[$lesson->field_lesson_number_value] = $lesson->field_lesson_number_value;
  }

  $last_lesson = end($options);
  $last_lesson = key($options);

  $first_lesson = reset($options);
  $first_lesson = key($options);

  $form['search_by_lessons']['lessons_from'] = array(
    '#type'          => 'select',
    '#options'       => $options,
    '#default_value' => $first_lesson,
  );

  $form['search_by_lessons']['lessons_till'] = array(
    '#type'          => 'select',
    '#options'       => $options,
    '#default_value' => $last_lesson,
  );

  $form['submit'] = array(
    '#type'       => 'submit',
    '#value'      => t('Search'),
    '#attributes' => array('class' => array('search-button')),
    '#ajax'       => array(
      'callback' => 'ditoolsi_training_pupil_inform_search_ajax_callback',
      'wrapper'  => 'inform-pupils',
    ),
    '#submit'     => array('ditoolsi_training_inform_pupils_search_submit'),
  );

  $query = ditoolsi_training_inform_pupils_query($course);
  if (isset($form_state['storage']['user_mail']) && $form_state['storage']['user_mail'] !== '') {
    $query->condition('u.mail',
      '%' . db_like($form_state['storage']['user_mail']) . '%', 'LIKE');
  }
  if (isset($form_state['storage']['date_from'])
    && $form_state['storage']['date_from'] !== ''
    && isset($form_state['storage']['date_till'])
    && $form_state['storage']['date_till'] !== ''
  ) {
    $date_from = strtotime($form_state['storage']['date_from']);
    $date_till = strtotime($form_state['storage']['date_till']);
    $query->condition('u.access', strtotime("midnight", $date_from), '>=');
    $query->condition('u.access', strtotime("tomorrow", $date_till) - 1, '<=');
  }
  if (isset($form_state['storage']['lessons_from'])
    && isset($form_state['storage']['lessons_till'])
  ) {
    $query->condition('pupils_list.current_lesson',
      $form_state['storage']['lessons_from'], '>=');
    $query->condition('pupils_list.current_lesson',
      $form_state['storage']['lessons_till'], '<=');
  }
  $results = $query->execute()->fetchAll();
  $header  = array(
    'number'        => t('№'),
    'user_name'     => t('Name'),
    'user_mail'     => t('E-mail'),
    'data_activity' => t('Date of activity'),
    'pupil_status'  => t('Status'),
    'lesson_pupil'  => t('№ lesson'),
  );

  $table_num   = 1;
  $table_users = array();
  foreach ($results as $u_key => $u_value) {
    $account                        = user_load($u_key);
    $name                           = ditoolsi_profile_name($account);
    $table_users[$u_value->user_id] = array(
      'number'        => $table_num,
      'user_name'     => $name,
      'user_mail'     => $u_value->mail,
      'data_activity' => date('d M Y', $u_value->access),
      'pupil_status'  => $u_value->status == 1 ? t('Active', array(),
        array('context' => 'Act')) : t('Blocked', array(),
        array('context' => 'Block')),
      'lesson_pupil'  => $u_value->current_lesson,
    );
    $table_num++;
  }

  $form['table'] = array(
    '#type'    => 'tableselect',
    '#header'  => $header,
    '#options' => $table_users,
    '#empty'   => t('No users found'),
  );

  $form['messages_content']['message_text'] = array(
    '#type'       => 'textarea',
    '#attributes' => array('class' => array('edit-messages-content-message-text-format')),
  );

  $form['messages_content']['message_mail'] = array(
    '#type'  => 'checkbox',
    '#title' => t('+email'),
  );

  $form['messages_content']['theme_container'] = array(
    '#type'   => 'container',
    '#states' => array(
      'invisible' => array(
        ':input[name="message_mail"]' => array('checked' => FALSE),
      ),
    ),
  );

  $form['messages_content']['theme_container']['message_mail_theme'] = array(
    '#type'      => 'textfield',
    '#title'     => t('Mail theme'),
    '#size'      => 60,
    '#maxlength' => 128,
  );

  $form['messages_content']['submit'] = array(
    '#type'       => 'submit',
    '#value'      => t('Send messages'),
    '#submit'     => array('ditoolsi_training_inform_pupils_send_submit'),
    '#attributes' => array('class' => array('send-button')),
    '#ajax'       => array(
      'callback' => 'ditoolsi_training_inform_pupils_send_ajax_callback',
      'wrapper'  => 'inform-pupils',
      // 'method'   => 'html',
      // 'effect'   => 'fade',
    ),
  );

  $form['#attributes'] = array('class' => array('results-pupil-form'));

  return $form;
}

function ditoolsi_training_inform_pupils_search_submit($form, &$form_state) {
  $form_state['storage']['user_mail']    = $form_state['values']['user_mail'];
  $form_state['storage']['status']       = $form_state['values']['date_search']['pupil_status'];
  $form_state['storage']['date_from']    = $form_state['values']['date_search']['date_from'];
  $form_state['storage']['date_till']    = $form_state['values']['date_search']['date_till'];
  $form_state['storage']['lessons_from'] = $form_state['values']['search_by_lessons']['lessons_from'];
  $form_state['storage']['lessons_till'] = $form_state['values']['search_by_lessons']['lessons_till'];
  $form_state['rebuild']                 = TRUE;
}

function ditoolsi_training_pupil_inform_search_ajax_callback(
  $form,
  $form_state
) {
  return $form;
}

/**
 * Function for list pupils lesson query.
 */
function ditoolsi_training_inform_pupils_lesson_query($node) {
  $query = db_select('field_data_field_lesson_course', 'lr');
  $query->innerJoin('field_data_field_lesson_number', 'lw',
    'lw.entity_id = lr.entity_id');
  $query->fields('lr', array('entity_id'));
  $query->fields('lw', array('field_lesson_number_value'));
  $query->condition('lr.field_lesson_course_target_id', $node->nid);
  $query->orderBy('lw.field_lesson_number_value', 'asc');

  return $query;
}

/**
 * Submit message to inform pupil page.
 */
function ditoolsi_training_inform_pupils_send_submit($form, &$form_state) {
  global $language;
  global $user;
  if (current($form_state['values']['table']) == 0) {
    drupal_set_message(t('Select users'), 'error', FALSE);
  }
  else {
    $body      = $form_state['values']['message_text'];
    $send_mail = $form_state['values']['message_mail'];
    $mail_from = variable_get('site_mail');
    $subject   = $form_state['values']['message_mail_theme'];
    $user_id   = $form_state['values']['table'];
    foreach ($user_id as $key => $value) {
      if ($value > 0) {
        $user_pupil = user_load($key);
        if (!$user_pupil) {
          continue;
        }
        ditoolsi_training_create_private_msg($user, $user_pupil, $body);

        if ($send_mail) {
          $recipient_message = $user_pupil->mail;
          ditoolsi_email_send('ditoolsi_training', 'pupil_send_message',
            $recipient_message, $language, array(
              'body'    => $body,
              'subject' => $subject
            ), $mail_from, $send = TRUE);
        }
      }
    }
    drupal_set_message(t('All messages were sent'), 'status', FALSE);
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Form callback to inform pupil page.
 */
function ditoolsi_training_inform_pupils_send_ajax_callback(
  $form,
  $form_state
) {

  return $form;
}

/**
 * Function for creation dialog and sending msg
 */
function ditoolsi_training_create_private_msg(
  $usr_from,
  $usr_to,
  $message_text
) {
  $usr_from = $usr_from->uid;
  $usr_to   = $usr_to->uid;

  $query = db_select('pm_index', 'i');
  $query->join('pm_message', 'm', 'i.mid = m.mid');
  $or = db_or();

  $and = db_and();
  $and
    ->condition('i.recipient', $usr_from)
    ->condition('m.author', $usr_to);
  $or->condition($and);

  $and = db_and();
  $and
    ->condition('i.recipient', $usr_to)
    ->condition('m.author', $usr_from);
  $or->condition($and);


  $query->condition($or);
  $query->fields('i', array('thread_id'));
  $results = $query->execute()->fetchObject();

  if (!empty($results)) {
    $mid = db_insert('pm_message')
      ->fields(array(
        'author'    => $usr_from,
        'subject'   => '',
        'body'      => $message_text,
        'format'    => 'filtered_html',
        'timestamp' => time(),
      ))
      ->execute();

    $query = db_insert('pm_index')
      ->fields(array(
        'mid',
        'thread_id',
        'recipient',
        'is_new',
        'deleted',
        'type'
      ));
    $query->values(array(
      'mid'       => $mid,
      'thread_id' => $results->thread_id,
      'recipient' => $usr_from,
      'is_new'    => 0,
      'deleted'   => 0,
      'type'      => 'user',
    ));
    $query->values(array(
      'mid'       => $mid,
      'thread_id' => $results->thread_id,
      'recipient' => $usr_to,
      'is_new'    => 1,
      'deleted'   => 0,
      'type'      => 'user',
    ));
    $query->execute();
  }
  else {
    $mid = db_insert('pm_message')
      ->fields(array(
        'author'    => $usr_from,
        'subject'   => '',
        'body'      => $message_text,
        'format'    => 'filtered_html',
        'timestamp' => time(),
      ))
      ->execute();

    $query = db_insert('pm_index')
      ->fields(array(
        'mid',
        'thread_id',
        'recipient',
        'is_new',
        'deleted',
        'type'
      ));
    $query->values(array(
      'mid'       => $mid,
      'thread_id' => $mid,
      'recipient' => $usr_from,
      'is_new'    => 0,
      'deleted'   => 0,
      'type'      => 'user',
    ));
    $query->values(array(
      'mid'       => $mid,
      'thread_id' => $mid,
      'recipient' => $usr_to,
      'is_new'    => 1,
      'deleted'   => 0,
      'type'      => 'user',
    ));
    $query->execute();
  }
}

/**
 * Function for inform pupils query.
 */
function ditoolsi_training_inform_pupils_query($node) {
  $params = array(
    ':entity_type' => 'user',
    ':bundle'      => 'user',
  );
  $type   = 'course';
  $query  = db_select('node', 'n');
  $query->leftJoin('ditoolsi_training_pupils_list', 'pupils_list',
    'pupils_list.course_id = n.nid');
  $query->innerJoin('users', 'u', 'u.uid = pupils_list.user_id');
  $query->fields('u', array('mail', 'access'));
  $query->fields('pupils_list', array(
    'user_id',
    'created',
    'status',
    'current_lesson'
  ));
  $query->condition('n.nid', $node->nid);
  $query->condition('n.type', $type);

  return $query;
}

/**
 * Build form to rule.
 */
function ditoolsi_training_messages_rules_form($form, &$form_state, $course) {
  $course = $form_state['build_info']['args'][0];

  $form['#id']           = 'notify-rules';
  $form['rule_fieldset'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Сreate Rule'),
    '#collapsible' => TRUE,
    '#collapsed'   => TRUE,
  );

  $form['rule_fieldset']['rule_title_item'] = array(
    '#type'   => 'item',
    '#markup' => '<h4 class="table-title">' . t('The list of rules and work with them') . '</h4>',
  );

  $form['rule_fieldset']['rules_tree'] = array(
    '#tree'   => TRUE,
    '#prefix' => '<div class="rules-messages">',
    '#suffix' => '</div>',
  );

  $status_options = array();
  $vocabulary     = taxonomy_vocabulary_machine_name_load('notify_events');
  $terms          = entity_load('taxonomy_term', FALSE,
    array('vid' => $vocabulary->vid));

  foreach ($terms as $key => $value) {
    $status_options[$key] = $value->name;
  }

  $form['rule_fieldset']['rules_tree']['status'] = array(
    '#type'          => 'select',
    '#title'         => t('Events'),
    '#options'       => $status_options,
    '#default_value' => DT_TRAINING_TT_DELETE,
  );

  for ($i = 1; $i < 31; $i++) {
    $days[] = $i;
  }

  $form['rule_fieldset']['rules_tree']['days'] = array(
    '#type'          => 'select',
    '#title'         => t('days'),
    '#options'       => $days,
    '#default_value' => 1,
    '#states'        => array(
      'visible' => array(
        ':input[name="rules_tree[status]"]' => array(
          'value' => DT_TRAINING_TT_NO_ACTIVITY
        ),
      ),
    ),
  );

  $form['rule_fieldset']['rules_tree_content'] = array(
    '#type'   => 'container',
    '#tree'   => TRUE,
    '#prefix' => '<div class="rules-messages-content">',
    '#suffix' => '</div>',
  );

  $form['rule_fieldset']['rules_tree_content']['title'] = array(
    '#type'       => 'textfield',
    '#attributes' => array('placeholder' => array(t('Name the rule'))),
    '#size'       => 60,
    '#maxlength'  => 20,
    '#required'   => TRUE,
  );

  $form['rule_fieldset']['rules_tree_content']['body'] = array(
    '#title'  => t('Mail messagge'),
    '#type'   => 'text_format',
    '#format' => 'filtered_html',
  );

  $form['rule_fieldset']['rules_tree_content']['vk'] = array(
    '#type'         => 'checkbox',
    '#title'        => t('Notify to vk'),
    '#return_value' => 1,
    '#access'       => defined('DT_TRAINING_PROMO_COURSE_' . $course->nid),
    '#states'       => array(
      'visible' => array(
        ':input[name="rules_tree[status]"]' => array(
          'value' => DT_TRAINING_TT_NO_ACTIVITY
        ),
      ),
    ),
  );

  $form['rule_fieldset']['rules_tree_content']['submit'] = array(
    '#type'       => 'submit',
    '#value'      => t('Create rule'),
    '#attributes' => array('class' => array('create-rule-button')),
    '#validate'   => array('ditoolsi_training_messages_rules_validate'),
  );

  $query = db_select('node', 'n');
  $query->innerJoin('field_data_field_notification_type', 'ne',
    'n.nid = ne.entity_id');
  $query->innerJoin('taxonomy_term_data', 'tt',
    'ne.field_notification_type_tid = tt.tid');
  $query->innerJoin('field_data_field_notification_course', 'nc',
    'n.nid = nc.entity_id');
  $query->condition('n.type', 'notification');
  $query->condition('nc.field_notification_course_target_id ', $course->nid);
  $query->fields('n', array('nid', 'title', 'status'));
  $query->fields('tt', array('name'));
  $nodes = $query->execute()->fetchAll();

  $header = array(
    'number'       => t('№'),
    'rule_name'    => t('Rule'),
    'rules_status' => t('Status rules'),
    'rule_event'   => t('Event'),
  );

  $table_num   = 1;
  $table_rules = array();
  foreach ($nodes as $r_key => $r_value) {

    if ($r_value->status > 0) {
      $status = t('Included');
    }
    else {
      $status = t('Off');
    }
    $name_rule = $r_value->title;

    $table_rules[$r_value->nid] = array(
      'number'       => $table_num,
      'rule_name'    => $name_rule,
      'rules_status' => $status,
      'rule_event'   => $r_value->name,
    );

    $table_num++;
  }
  $form['rule_fieldset']['table'] = array(
    '#type'    => 'tableselect',
    '#header'  => $header,
    '#options' => $table_rules,
    '#empty'   => t('No rules found'),
  );

  $form['rule_fieldset']['activate_rules'] = array(
    '#type'   => 'submit',
    '#value'  => t('Switch on'),
    '#ajax'   => array(
      'callback' => 'ditoolsi_training_rules_includes_submit',
      'wrapper'  => 'notify-rules',
    ),
    '#submit' => array('ditoolsi_training_rules_includes_ajax_callback'),
  );

  $form['rule_fieldset']['block_pupils'] = array(
    '#type'   => 'submit',
    '#value'  => t('Turn off'),
    '#ajax'   => array(
      'callback' => 'ditoolsi_training_rules_off_submit',
      'wrapper'  => 'notify-rules',
    ),
    '#submit' => array('ditoolsi_training_rules_off_ajax_callback'),
  );

  return $form;
}

function ditoolsi_training_messages_rules_validate($form, &$form_state) {
  $values = $form_state['values'];
  $course = $form_state['build_info']['args'][0];
  if ($values['rules_tree']['status'] == DT_TRAINING_TT_NO_ACTIVITY
    && empty($values['rules_tree']['days'])
  ) {
    form_set_error('rules_tree][days', t('!name field is required.', array(
      '!name' => t('Days count'),
    )));
  }

  $query = db_select('node', 'n');
  $query->innerJoin('field_data_field_notification_type', 'ne',
    'n.nid = ne.entity_id');
  $query->innerJoin('field_data_field_notification_course', 'nc',
    'n.nid = nc.entity_id');
  $query->condition('n.type', 'notification');
  $query->condition('ne.field_notification_type_tid',
    $values['rules_tree']['status']);
  $query->condition('nc.field_notification_course_target_id ', $course->nid);
  $query->fields('n', array('nid'));
  $nodes = $query->execute()->fetchAll();

  if (!empty($nodes)) {
    form_set_error('rules_tree][status',
      t('You have already event with this type'));
  }
}

function ditoolsi_training_messages_rules_form_submit($form, &$form_state) {
  global $user;

  $values = $form_state['values'];
  $course = $form_state['build_info']['args'][0];
  $status = $values['rules_tree']['status'];

  $notification = entity_create('node', array(
    'type' => 'notification',
    'uid'  => $user->uid,
  ));

  $n_wrapper                            = entity_metadata_wrapper('node',
    $notification);
  $n_wrapper->title                     = $values['rules_tree_content']['title'];
  $n_wrapper->body                      = $values['rules_tree_content']['body'];
  $n_wrapper->field_notification_course = $course->nid;
  $n_wrapper->field_notification_type   = $status;
  $n_wrapper->field_days_of_inactivity  = $values['rules_tree']['days'];
  $n_wrapper->field_notification_vk     = $values['rules_tree_content']['vk'];
  $n_wrapper->save();

  $nid = $n_wrapper->getIdentifier();

  if ($nid) {
    drupal_set_message(t('Notification has been created'));
  }
  else {
    drupal_set_message(t('An error occurred'));
  }
  $form_state['redirect'] = 'course/' . $course->nid . '/send-messages';
}

function ditoolsi_training_messages_rules_form_ajax_callback(
  $form,
  &$form_state
) {
  return $form;
}

/**
 * Form callback include rule to inform pupil page.
 */
function ditoolsi_training_rules_includes_submit($form, $form_state) {
  $sum = array_sum($form_state['values']['table']);
  if (empty($sum)) {
    drupal_set_message(t('Select rule'), 'error');
  }
  else {
    drupal_set_message(t('All rules activate'));
  }

  return $form;
}

/**
 * Form submit include rule to inform pupil page.
 */
function ditoolsi_training_rules_includes_ajax_callback($form, &$form_state) {
  $rules_id = array_unique($form_state['values']['table']);
  if (!empty($rules_id)) {
    foreach ($rules_id as $r_key => $r_value) {
      $node         = node_load($r_key);
      $node->status = NODE_PUBLISHED;
      node_save($node);
    }
  }
  $form['rebuild'] = TRUE;
}

/**
 * Form submit off rule to inform pupil page.
 */
function ditoolsi_training_rules_off_submit($form, $form_state) {
  $sum = array_sum($form_state['values']['table']);

  if (empty($sum)) {
    drupal_set_message(t('Select rule'), 'error', FALSE);
  }
  else {
    drupal_set_message(t('Checked rules was disabled'), 'error', FALSE);
  }

  return $form;
}

/**
 * Form callback off rule to inform pupil page.
 */
function ditoolsi_training_rules_off_ajax_callback($form, &$form_state) {
  $rules_id = array_unique($form_state['values']['table']);
  if (!empty($rules_id)) {
    foreach ($rules_id as $r_key => $r_value) {
      $node         = node_load($r_key);
      $node->status = NODE_NOT_PUBLISHED;
      node_save($node);
    }
  }

  $form['rebuild'] = TRUE;
}

function ditoolsi_training_course_blocked_form($form, &$form_state) {
  $form = array();

  $course       = node_load($form_state['build_info']['args'][0]);
  $teacher      = user_load($course->uid);
  $teacher_name = ditoolsi_profile_name($teacher->uid);

  $query = db_select('ditoolsi_training_curators_list', 'tcl');
  $query->condition('tcl.course_id', $course->nid);
  $query->fields('tcl', array('course_id', 'user_id'));
  $result  = $query->execute()->fetchAll();
  $curator = array();
  foreach ($result as $key => $value) {
    $curator[] = user_load($value->user_id);
  }


  $blocked_text = t('This course is blocked for you. Please contact your teacher or curator');
  $form['text'] = array(
    '#type'   => 'item',
    '#markup' => '<div class="text">' . $blocked_text . '</div>',
  );

  $form['teacher'] = array(
    '#type'   => 'item',
    '#title'  => t('Teacher:'),
    '#markup' => '<div class="text">' . l($teacher_name,
        'user/' . $teacher->uid) . '</div>'
  );

  if (count($curator)) {
    $form['curators'] = array(
      '#type'  => 'item',
      '#title' => t('Curators:'),
    );
    foreach ($curator as $key => $value) {
      $curator_name       = ditoolsi_profile_name($value->uid);
      $form['curators'][] = array(
        '#type'   => 'item',
        '#markup' => '<div class="text">' . l($curator_name,
            'user/' . $value->uid) . '</div>'
      );
    }
  }

  return $form;
}

function ditoolsi_training_first_session_form($form, &$form_state) {
  $form['#tree'] = TRUE;
  $variable      = variable_get('training_first_session', array());

  if (isset($variable['course'])) {
    $node  = node_load($variable['course']);
    $title = isset($node->title) ? $node->title : '';
  }
  else {
    $title = '';
  }

  $form['training_first_session']['video'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Training video for first session'),
    '#required'      => TRUE,
    '#default_value' => isset($variable['video']) ? $variable['video'] : '',
  );

  return system_settings_form($form);
}

function ditoolsi_training_configure_activate_mail_form(
  $form,
  &$form_state,
  $node
) {
  $mail   = ditoolsi_training_get_mail_activation($node);
  $titles = array(
//    t('Message of activation account (student registration)'),
    t('Message of activation account (manual student registration)'),
    t('Adding student'),
  );

  foreach ($titles as $key => $title) {
    $form['fieldset'][$key] = array(
      '#title' => $title,
      '#type'  => 'fieldset',
    );

    $form['fieldset'][$key]['subject'] = array(
      '#title'         => t('Subject'),
      '#type'          => 'textfield',
      '#default_value' => $mail[$key]['subject'],
    );

    $form['fieldset'][$key]['body'] = array(
      '#title'         => t('Body'),
      '#type'          => 'text_format',
      '#format'        => 'plain_text',
      '#default_value' => $mail[$key]['body'],
    );
  }

  $form['actions']['submit'] = array(
    '#type'       => 'submit',
    '#value'      => t('Save'),
    '#submit'     => array('ditoolsi_training_configure_activate_mail_form_submit_ajax'),
    '#ajax'       => array(
      'callback' => 'ditoolsi_training_configure_activate_mail_form_ajax',
      'event'    => 'click',
      'wrapper'  => 'training-configure-mail-form',
    ),
    '#attributes' => array(
      'class' => array('dit-submit'),
    ),
  );

  $form['#after_build'][] = 'ditoolsi_training_configure_activate_mail_form_after_build';
  $form['#id']            = 'training-configure-mail-form';
  $form['#tree']          = TRUE;
  $form['#attributes']    = array(
    'class' => array(
      'process-ditools-labels',
    ),
  );

  form_load_include($form_state, 'module', 'ditoolsi_training',
    'includes/form');

  return $form;
}

function ditoolsi_training_configure_activate_mail_form_after_build(
  $form,
  &$form_state
) {
  foreach ($form['fieldset'] as $key => &$fields) {
    if (isset($fields['body']['format'])) {
      $fields['body']['format']['#access'] = FALSE;
    }
  }

  return $form;
}

function ditoolsi_training_configure_activate_mail_form_ajax(
  $form,
  $form_state
) {
  return $form;
}

function ditoolsi_training_configure_activate_mail_form_submit_ajax(
  $form,
  &$form_state
) {
  $node                            = $form_state['build_info']['args'][0];
  $node->field_course_mail_subject = array(
    LANGUAGE_NONE => array(
      0 => array(
        'value' => $form_state['values']['fieldset'][0]['subject'],
      ),
    ),
  );

  $node->field_course_mail_body = array(
    LANGUAGE_NONE => array(
      0 => array(
        'value'  => $form_state['values']['fieldset'][0]['body']['value'],
        'format' => $form_state['values']['fieldset'][0]['body']['format'],
      ),
    ),
  );

  $node->field_course_mail_adding_subject = array(
    LANGUAGE_NONE => array(
      0 => array(
        'value' => $form_state['values']['fieldset'][1]['subject'],
      ),
    ),
  );

  $node->field_course_mail_adding_body = array(
    LANGUAGE_NONE => array(
      0 => array(
        'value'  => $form_state['values']['fieldset'][1]['body']['value'],
        'format' => $form_state['values']['fieldset'][1]['body']['format'],
      ),
    ),
  );

  node_save($node);
  drupal_set_message(t('Changes was saved'));
}

function ditoolsi_training_library_buy_form($form, &$form_state, $node) {
  $cost = field_get_items('node', $node, 'field_library_cost');

  foreach (field_get_items('node', $node, 'field_library_courses') as $value) {
    $course  = node_load($value['target_id']);
    $balance = ditoolsi_bonuses_get_course_bonus_balance($course);

    if ((int) $balance && $balance >= $cost[0]['value']) {
      break;
    }
    else {
      unset($course);
    }
  }

  if (empty($course)) {
    drupal_set_message(t('Your account is not enough bonus points'), 'error');
    drupal_goto('library/' . $node->nid);
  }

  $form_state['course'] = $course;
  $form_state['cost']   = $cost[0]['value'];

  $form['actions'] = array(
    '#type' => 'actions',
  );

  $form['actions']['submit'] = array(
    '#type'       => 'submit',
    '#value'      => t('Buy'),
    '#attributes' => array(
      'class' => array('dit-submit'),
    ),
    '#prefix'     => '<div><b>' . t('Your account will be debited on the course !course !points bonus points',
        array(
          '!course' => l($course->title, 'node/' . $course->nid),
          '!points' => $cost[0]['value'],
        )) . '</b></div>',
  );

  $form['actions']['cancel'] = array(
    '#markup' => l(t('Cancel'), 'library/' . $node->nid, array(
      'attributes' => array(
        'class' => array('cancel-link'),
      ),
    )),
  );

  return $form;
}

function ditoolsi_training_library_buy_form_submit($form, &$form_state) {
  global $user;

  $node        = $form_state['build_info']['args'][0];
  $account     = ditoolsi_profile_account($user, FALSE);
  $transaction = db_transaction();

  try {
    ditoolsi_bonuses_upd_course_bonus_balance($account,
      $form_state['course']->nid, $form_state['cost'] * -1);
    db_insert('ditoolsi_training_library_pupils')
      ->fields(array(
        'user_id'    => $user->uid,
        'library_id' => $node->nid,
        'created'    => REQUEST_TIME,
      ))
      ->execute();
    drupal_set_message(t('You have become a party to the library'));
  } catch (Exception $e) {
    $transaction->rollback();
    drupal_set_message(t('An error occurred'), 'error');
  }

  $form_state['redirect'] = 'library/' . $node->nid;
}

function mgc_send_mail($mail, $mail_subject, $mail_body) {

  $headers = 'MIME-Version: 1.0' . "\r\n";
  $headers .= 'Content-Type: text/html; charset=UTF-8; format=flowed' . "\r\n";
  $headers .= 'From: ' . variable_get('site_mail',
      'noreply@up-lvl.ru') . "\r\n";
  $headers .= "X-Mailer: PHP/" . phpversion();
  mail($mail, $mail_subject, nl2br($mail_body), $headers,
    "-f " . variable_get('site_mail', 'noreply@up-lvl.ru'));
}


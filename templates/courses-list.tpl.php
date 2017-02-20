<?php
$query  = ditoolsi_training_get_cources_query();
$result = $query->execute()->fetchCol();

$query     = ditoolsi_training_get_cources_query($user->uid, FALSE);
$available = $query->execute()->fetchCol();
$teacher   = ditoolsi_training_get_teacher();

if (ditoolsi_training_access('add course')):
  ?>
    <div class="block-add-course">
        <div class="image-add-course"><a href="course/add"></a></div>
        <div class="text-add-course"><a
                    href="course/add"><?php print(t('Add course')); ?></a></div>
    </div>
  <?php
endif;

if ($teacher && $teacher->uid == DITOOLSI_FEEDBACK_UID) {
  $page = 'courses';

  if (isset($teacher->data['_banner_' . md5($page)])) {
    $image   = $teacher->data['_banner_' . md5($page)]['image'];
    $url     = $teacher->data['_banner_' . md5($page)]['url'];
    $enabled = $teacher->data['_banner_' . md5($page)]['enabled'];
  }
  else {
    $image   = 0;
    $url     = '';
    $enabled = 0;
  }

  $file = file_load($image);

  if ($enabled && $file) {
    print '<div id="banner">' . l(theme('image', array(
        'path' => $file->uri,
      )), $url, array(
        'html'       => TRUE,
        'attributes' => array(
          'target' => '_blank',
        ),
      )) . '</div>';
  }


  if ($user->uid == DITOOLSI_FEEDBACK_UID) {
    print l('Редкатировать баннер', 'edit-banner', array(
      'query'      => array(
        'destination' => $page,
      ),
      'attributes' => array(
        'class' => array('edit-banner-link'),
      ),
    ));
  }
}
?>
<div class="courses-list">
  <?php

  if ($items) {
    $courses = array(
      array(),
      array(),
    );

    foreach ($items as $item) {
      if (isset($item->node->field_course_card_type[LANGUAGE_NONE][0]['value']) && $item->node->field_course_card_type[LANGUAGE_NONE][0]['value'] == DITOOLSI_TRAINING_COURSE_CARD_2) {
        $hook = 'ditoolsi_training_course_list_item2';
      }
      else {
        $hook = 'ditoolsi_training_course_list_item';
      }

      if (in_array($item->node->nid, $available)) {
        $courses[0][] = theme($hook, array(
          'node'        => $item->node,
          'picture'     => $item->picture,
          'title'       => $item->title,
          'description' => $item->description,
          'type'        => $item->type,
          'wrapper'     => $item->wrapper,
          'pupils'      => $item->pupils,
          'blocked'     => $item->blocked,
          'result'      => $result,
        ));
      }
      else {
        $courses[1][] = theme($hook, array(
          'node'        => $item->node,
          'picture'     => $item->picture,
          'title'       => $item->title,
          'description' => $item->description,
          'type'        => $item->type,
          'wrapper'     => $item->wrapper,
          'pupils'      => $item->pupils,
          'blocked'     => $item->blocked,
          'result'      => $result,
        ));
      }
    }

    if (isset($user->roles[DITOOLSI_PUPIL_RID])) {
      array_unshift($courses[0],
        '<h3><b>Доступные для прохождения курсы</b></h3><br>');

      if ($courses[1]) {
        array_unshift($courses[1],
          '<hr><br><h3><b>Вы можете приобрести эти курсы</b></h3><br>');
      }
    }

    print implode($courses[0]);
    print implode($courses[1]);
  }
  else {
    //print '<div class="empty-text">' . t('You have no available courses') .
    // '</div>';
    if (mgc_user_has_teacher_role()) {
      $teach_course = node_load(MGC_TEACH_COURSE_ID);
      $item         = new stdClass();

      $item->wrapper = entity_metadata_wrapper('node', $teach_course);
      $item->node    = $teach_course;
      $item->picture = $item->wrapper->field_course_image->value();
      $item->title   = $teach_course->title;

      $description       = $item->wrapper->body->value();
      $item->description = check_markup($description['value'],
        $description['format']);
      $item->type        = $item->wrapper->field_course_type->value();
      $item->pupils      = 0;
      $item->blocked     = FALSE;


      print theme('mgc_teach_course_list_item', array(
        'node'        => $item->node,
        'picture'     => $item->picture,
        'title'       => $item->title,
        'description' => $item->description,
        'type'        => $item->type,
        'wrapper'     => $item->wrapper,
        'pupils'      => $item->pupils,
        'blocked'     => $item->blocked,
        'result'      => array(),
      ));
    }
  }
  ?>
</div>

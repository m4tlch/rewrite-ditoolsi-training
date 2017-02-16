<?php
$query = ditoolsi_training_get_cources_query();
$result = $query->execute()->fetchCol();
?>
<div class="course-image">
  <?php
    if (!empty($picture['uri'])) {
      print theme('image_style', array(
        'style_name' => '300x170sc',
        'path' => $picture['uri'],
      ));
    }
  ?>
</div>
<?php if (!in_array($wrapper->getIdentifier(), $result) || user_access('training view courses teacher')): ?>
<div class="requirements clearfix">
  <?php
    switch ($type) {
      case DITOOLSI_TRAINING_COURSE_TYPE_PAID:
        print '<div class="money">' . t('@rub rub.', array(
          '@rub' => $wrapper->field_course_cost->value(),
        )) . '</div>';
        break;
    }
  ?>
</div>
<?php endif; ?>
<div class="course-info clearfix">
  <?php if (user_access('training view courses teacher')): ?>
  <div class="info-line clearfix">
    <label>
      <?php
        print t('Specialization');
      ?>:
    </label>
    <div class="value">
      <?php
        print $wrapper->field_course_specialization->value();
      ?>
    </div>
  </div>

  <div class="info-line clearfix">
    <label>
      <?php
      print t('Time for completion');
      ?>:
    </label>
    <div class="value">
      <?php
      $days    = $wrapper->field_course_time_completion->value();
      $context = ditoolsi_prular($days);

      if ($days > 1) {
        print t('!count days', array(
          '!count' => $days,
        ), array(
          'context' => $context,
        ));
      }
      else {
        print t('1 day');
      }
      ?>
    </div>
  </div>
  <?php endif; ?>

  <div class="info-line clearfix">
    <?php
    if (ditoolsi_training_access('request learning', $wrapper->value())) {
      if (isset($node->field_course_payment_link[LANGUAGE_NONE][0]['value'])) {
        $url = $node->field_course_payment_link[LANGUAGE_NONE][0]['value'];
      } else {
        $url = 'course/' . $wrapper->getIdentifier() . '/request';
      }

      print l(t('Send request and buy course'), $url, array(
        'attributes' => array(
          'class' => array('btn dit-submit'),
          'data-course-id' => $wrapper->getIdentifier(),
        ),
      ));
    } elseif (ditoolsi_training_request_sent($wrapper->value())) {
      print '<div class="request-sent">' . t('Request sent') . '</div>';
    }
    ?>
  </div>

  <?php
  $group_url = $wrapper->field_course_sn_group->value();

  if ($group_url && ($wrapper->field_course_sn_group_visible->value() || user_access('training view courses teacher'))) {
    print '<div class="group-link">';
    print l($group_url, $group_url, array(
      'attributes' => array(
        'target' => '_blank',
      ),
      'absolute' => TRUE,
    ));
    print '</div>';
  }
  ?>
</div>

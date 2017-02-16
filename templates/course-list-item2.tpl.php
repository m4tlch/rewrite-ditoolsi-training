<?php
$blocked_attr = array();

if ($blocked) {
  ditoolsi_training_blocked_popup_style();
  $course_path = 'nojs/' . $node->nid . '/blocked';
  $blocked_attr = array('class'=>array('ctools-use-modal', 'ctools-modal-blocked-popup-style'));
} else {
  $course_path = $node->nid;
}
if (isset($node->field_course_card_background[LANGUAGE_NONE][0]['uri'])) {
  $bg = 'url(' . file_create_url($node->field_course_card_background[LANGUAGE_NONE][0]['uri']) . ')';
} else {
  $bg = '#ccc';
}

$wrapper = entity_metadata_wrapper('node', $node);
?>
<div class="course-list-item type-2 clearfix" data-id="<?= $node->nid; ?>" style="background: <?= $bg; ?>; cursor: pointer;" data-url="<?= url("node/{$node->nid}"); ?>">
  <div class="picture">
    <?php if (user_access('training save order courses')): ?>
      <div class="arrows">
        <i class="fa fa-arrow-up up"></i>
        <i class="fa fa-arrow-down down"></i>
      </div>
    <?php endif; ?>
  </div>

  <?php if ($wrapper->value()->uid != DITOOLSI_FEEDBACK_UID): ?>
    <?php if ($wrapper->field_course_specialization->value()): ?>
    <div class="specialization">
      <?php
      print $wrapper->field_course_specialization->value();
      ?>
    </div>
    <?php endif; ?>
    <div class="lessons">
      <?php
      $count = count($wrapper->value()->lessons);
      print t('@count lessons', array(
        '@count' => $count,
      ), array(
        'context' => ditoolsi_prular($count),
      ));
      ?>
    </div>
    <div class="cost">
      <?php
      switch ($type) {
        case DITOOLSI_TRAINING_COURSE_TYPE_PAID:
          print '<span>Стоимость</span>' . number_format($wrapper->field_course_cost->value(), 0, ' ', ' ');
          break;

        default:
          print 'Бесплатно';
          break;
      }
      ?>
    </div>
    <div class="title">
      <?= l($node->title, "node/{$node->nid}"); ?>
    </div>
  <?php endif; ?>
  <?php
  if (ditoolsi_training_access('request learning', $node)) {
    if (isset($node->field_course_payment_link[LANGUAGE_NONE][0]['value'])) {
      $url = $node->field_course_payment_link[LANGUAGE_NONE][0]['value'];
    } else {
      $url = 'course/' . $node->nid . '/request';
    }

    print '<div class="request-link">' . l(t('Send request and buy course'), $url, array(
      'attributes' => array(
        'class' => array('btn dit-submit'),
        'data-course-id' => $node->nid,
      ),
    )) . '</div>';
  } elseif (ditoolsi_training_request_sent($node)) {
    print '<div class="request-link"><div class="request-sent">' . t('Request sent') . '</div></div>';
  }
  ?>
</div>

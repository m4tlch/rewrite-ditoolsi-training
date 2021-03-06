<?php
$blocked_attr = array();

if ($blocked) {
  ditoolsi_training_blocked_popup_style();
  $course_path = 'nojs/' . $node->nid . '/blocked';
  $blocked_attr = array('class'=>array('ctools-use-modal', 'ctools-modal-blocked-popup-style'));
} else {
  $course_path = $node->nid;
}
?>
<div class="course-list-item type-1 clearfix" data-id="<?= $node->nid; ?>">
  <div class="picture">
    <?php if (user_access('training save order courses')): ?>
    <div class="arrows">
      <i class="fa fa-arrow-up up"></i>
      <i class="fa fa-arrow-down down"></i>
    </div>
    <?php
    endif;
    print l(theme('image_style', array(
      'style_name' => '300x170sc',
      'path'       => $picture['uri'],
    )), 'course/' . $course_path, array(
      'html' => TRUE,
      'attributes' => $blocked_attr,
    ));
    ?>
  </div>
  <div class="info">
    <div class="title">
      <?php
      print l($title, 'course/' . $course_path, array('attributes' => $blocked_attr));
      ?>
    </div>
    <div class="body">
      <?php
      $alter = array(
        'max_length'    => 205,
        'word_boundary' => TRUE,
        'ellipsis'      => TRUE,
        'html'          => TRUE,
      );

      print views_trim_text($alter, $description);
      ?>
    </div>
    <div class="pupils">
      <?php
      if (!empty($pupils)) {
        print t('Pupils: !count', array(
          '!count' => '<span>' . $pupils . '</span>',
        ));
      }
      ?>
    </div>

    <div class="request">
      <?php if (!in_array($node->nid, $result) || user_access('training view courses teacher')): ?>
        <div class="requirements clearfix">
          <?php
          switch ($type) {
            case DITOOLSI_TRAINING_COURSE_TYPE_PAID:
              if (!empty($node->field_course_cost[LANGUAGE_NONE][0]['value'])) {
                print '<div class="money">' . t('@rub rub.', array(
                    '@rub' => $node->field_course_cost[LANGUAGE_NONE][0]['value'],
                  )) . '</div>';
              }
              break;

            default:
              print '<div>Бесплатно</div>';
              break;
          }
          ?>
        </div>
      <?php endif; ?>

      <?php
      if (ditoolsi_training_access('request learning', $node)) {
        if (isset($node->field_course_payment_link[LANGUAGE_NONE][0]['value'])) {
          $url = $node->field_course_payment_link[LANGUAGE_NONE][0]['value'];
        } else {
          $url = 'course/' . $node->nid . '/request';
        }

        print l(t('Send request and buy course'), $url, array(
          'attributes' => array(
            'class' => array('btn dit-submit'),
            'data-course-id' => $node->nid,
          ),
        ));
      } elseif (ditoolsi_training_request_sent($node)) {
        print '<div class="request-sent">' . t('Request sent') . '</div>';
      }
      ?>
    </div>
  </div>
</div>

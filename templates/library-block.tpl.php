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
<div class="course-info clearfix">
  <div class="info-line clearfix">
    <label>
      <?php
        print t('Library for courses');
      ?>:
    </label>
    <div class="value">
      <?php
        print implode(', ', $courses);
      ?>
    </div>
  </div>
</div>
<?php
  if (!ditoolsi_training_library_is_available($node) && !defined('DT_TRAINING_PROMO_COURSE_LIB_' . $node->nid)) {
    print l(t('Buy library'), 'library/' . $node->nid . '/buy', array(
      'attributes' => array(
        'class' => array(
          'btn dit-submit',
        ),
      ),
      'query' => array(
        'token' => drupal_get_token('buy-library-' . $node->nid . '-' . session_id()),
      ),
    ));
  }
?>
<div class="course-list-item clearfix">
  <div class="picture">
    <?php
      print l(theme('image_style', array(
        'style_name' => '300x170sc',
        'path'       => $picture['uri'],
      )), 'library/' . $node->nid, array(
        'html' => TRUE,
      ));
    ?>
  </div>
  <div class="info">
    <div class="title">
      <?php
        print l($title, 'library/' . $node->nid);
      ?>
    </div>
    <div class="courses">
      <?php
        print t('Library for courses - !name', array(
          '!name' => '<span>' . $courses . '</span>',
        ));
      ?>
    </div>
    <div class="cost">
      <?php
        if (ditoolsi_training_library_is_available($node)) {
          print '<b>' . t('Paid', array(), array('context' => 'library')) . '</b>';
        } else {
          if (!$cost) {
            $cost = t('Free');
          }

          print t('Cost libraries - !count', array(
            '!count' => '<span>' . $cost . '</span>',
          ));
        }
      ?>
    </div>
  </div>
</div>
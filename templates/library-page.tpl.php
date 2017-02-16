<?php
$libraries = ditoolsi_training_get_libraries();
drupal_add_library('system', 'drupal.ajax');
?>
    <div class="all-courses-dropdown dropdown">
        <div class="all-courses-btn dropdown-toggle" data-toggle="dropdown"
             id="list-courses-dropdown">
    <span>
      <i class="fa fa-bars"></i>
      <?php print t('All libraries'); ?>
    </span>
        </div>


        <ul class="dropdown-menu" role="menu" aria-labelledby="list-courses-dropdown">
            <li role="presentation">
              <?php
              print l(t('All libraries'), 'libraries', array(
                'attributes' => array(
                  'class'    => array('bold'),
                  'role'     => 'menuitem',
                  'tabindex' => -1,
                ),
              ));
              ?>
            </li>
          <?php
          foreach ($libraries as $library) {
            print '
          <li role="presentation">
            ' . l($library->title, 'library/' . $library->nid, array(
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

    <div class="course-block">
      <?php
      print theme('ditoolsi_training_library_block', array(
        'node'    => $node,
        'picture' => $wrapper->field_library_picture->value(),
        'courses' => $courses,
        'cost'    => $wrapper->field_library_cost->value(),
        'wrapper' => $wrapper,
      ));
      ?>
    </div>
    <div class="course-content">
        <div class="course-title">
            <div class="title">
              <?php
              print $node->title;
              ?>
            </div>
            <div class="course-id">
                id<?php print $node->nid; ?>
            </div>

        </div>
        <div class="course-body">
          <?php
          print $description;
          ?>
        </div>
    </div>
<?php
if ($page === TRUE):
  if (ditoolsi_training_access('edit library')):
    ?>
      <div class="links-library">
    <span class="edit-link-library">
          <?php
          print l(t('Edit', array(), array('context' => 'edit library')),
            'library/' . $node->nid . '/edit');
          ?>
    </span>
          <span class="delete-link-library">
          <?php
          print l(t('Delete'), 'library/' . $node->nid . '/delete');
          ?>
    </span>
          <span class="add-pupil-link-library">
          <?php
          print l(t('Users library'), 'library/' . $node->nid . '/add-pupils');
          ?>
    </span>
      </div>
    <?php
  endif;
  if (defined('DT_TRAINING_PROMO_COURSE_LIB_' . $node->nid)):
    $articles = ditoolsi_training_get_1afl_useful_articles();

    print '
          <div class="course-lessons">
          <div class="lessons-quantity">
            ' . t('Chain sections: !count', array(
        '!count' => '<span>' . count($articles) . '</span>',
      )) . '
          </div>
          <div id="useful-articles">
          <div class="pane-title">
            Полезные статьи
          </div>
          <div class="pane-content">';

    foreach ($articles as $article) {
      print theme('ditoolsi_training_useful_article', array(
        'node' => $article,
      ));
    }

    print '</div></div></div>';
  else:
    ?>
      <div class="course-lessons">
          <div class="lessons-quantity">
            <?php
            print t('Chain sections: !count', array(
              '!count' => '<span>' . count($sections) . '</span>',
            ));
            ?>
          </div>
          <div class="lessons-list">
            <?php
            $i = 0;
            foreach ($sections as $section) {
              $i++;
              print theme('ditoolsi_training_section_list_item', array(
                'number'  => $i,
                'title'   => $section->title,
                'section' => $section,
                'nid'     => $section->nid,
              ));
            }

            if (ditoolsi_training_access('add section')):
              ?>
                <div class="lesson-list-item add-lesson-item clearfix">
                  <?php
                  $html = '
              <div class="add-lesson-button">
                <span>
                  <i class="fa fa-plus"></i>
                </span>
              </div>';
                  print l($html, 'library/' . $node->nid . '/add-section', array(
                    'html' => TRUE,
                  ));
                  ?>
                    <div class="add-lesson-label">
                      <?php
                      print l(t('Add section'), 'library/' . $node->nid . '/add-section');
                      ?>
                    </div>
                </div>
              <?php
            endif;
            ?>
          </div>
          <div id='section'>

          </div>
      </div>
    <?php
  endif;
endif;
?>
<?php
  $wrapper = entity_metadata_wrapper('node', $section);
  $library_id = $wrapper->field_section_library->raw();
?>
<div class="lesson-list-item clearfix">
  <div class="lesson-number">
    <span>
      <?php
        print $number;
      ?>
    </span>
  </div>
  <div class="lesson-info">
    <div class="lesson-title">
      <?php
        if (ditoolsi_training_library_is_available(node_load($library_id))) {
          print l($section->title, 'library/' . $library_id . '/' . $section->nid, array(
            'attributes' => array(
              'class' => array(
                'use-ajax',
              ),
            ),
            'html' => TRUE,
          ));
        } else {
          print '<a>' . $section->title . '</a>';
        }
      ?>
      <span class="edit-link">
        <?php
          if (ditoolsi_training_access('edit library')) {
            print l('&nbsp;', 'library/' . $library_id . '/' . $section->nid . '/edit', array(
              'html'  => TRUE,
              'query' => array(
                'destination' => current_path(),
              ),
            ));
          }
        ?>
      </span>
    </div>
    <div class="lesson-stat">
      <?php
        $info = array();
        $files = isset($section->field_section_file[LANGUAGE_NONE]) ? $section->field_section_file[LANGUAGE_NONE] : array();
        $files_quantity = count($files);

        if ($files_quantity > 1) {
          $info[] = t('!count files', array(
            '!count' => $files_quantity,
          ));
        } elseif ($files_quantity == 1) {
          $info[] = t('1 file');
        }

        print implode(', ', $info);
      ?>
    </div>
  </div>
</div>

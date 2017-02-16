<?php
  if (ditoolsi_training_access('edit library')):
?>
  <div class="add-library">
    <div class="library-images">
      <?php
        print  l('&nbsp;', "node/add/library");
      ?>
    </div>
    <div class="library-text">
      <?php
        print  l(t('Add library'), "node/add/library");
      ?>
    </div>
  </div>
<?php
  endif;
?>
<div class="courses-list">
  <?php
    if ($items) {
      foreach ($items as $item) {
        print theme('ditoolsi_training_library_list_item', array(
          'node'        => $item->node,
          'picture'     => $item->picture,
          'title'       => $item->title,
          'courses'     => $item->courses,
          'cost'        => $item->cost,
        ));
      }
    }
    else {
      print '<div class="empty-text">' . t('You have no available libraries') . '</div>';
    }
  ?>
</div>
(function ($) {

  "use strict";

  Drupal.behaviors.ditoolsiTrainingSelectLesson = {
    attach: function () {
      $('.lessons-list .lesson-number')
        .once('ditoolsiTrainingSelectLesson', function () {
          $(this).bind('click', function () {
            var $item = $(this).parents('.lesson-list-item');

            $('.lessons-list')
              .find('.lesson-list-item')
              .not($item)
              .removeClass('selected');

            $item.toggleClass('selected');
          });
        });
    }
  };

})(jQuery);
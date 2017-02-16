(function ($) {
  Drupal.behaviors.ditoolsiTrainingCheckbox = {
    attach: function() {
      $('.table-select-processed .form-type-checkbox>input[type="checkbox"]').each(function () {
        if ($(this).is(':checked')) {
          $(this).parents('tr').addClass('selected');
        }
      });
    }
  };

  Drupal.behaviors.ditoolsiTrainingFormLabel = {
    attach: function() {
      $('#list-pupils-library')
        .find('.select-all .form-checkbox')
        .once('ditoolsiTrainingFormLabel', function () {
          var id = Math.random().toString().split('.')[1];

          $(this).attr('id', id);

          $('<label />', {
            for: id
          }).insertAfter($(this));
        });
    }
  };

})(jQuery);

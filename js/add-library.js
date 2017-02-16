(function ($) {

  "use strict";

  Drupal.behaviors.ditoolsiTrainingFileField = {
    attach: function() {
      $('.ditoolsi-training-file-field').find('input[type="file"]').once('ditoolsiTrainingFileField', function () {
        var $field = $(this),
            $button = $('<span />', {
              class: 'btn btn-default btn-file ditoolsi-training-file-field-btn'
            }).html(Drupal.t('Browse...')),
            $textField = $('<input />', {
              type: 'text',
              class: 'form-control',
              readonly: 'readonly'
            }),
            $group = $('<div />', {
              class: 'input-group-btn'
            }).append($button);
        $button.bind('click', function () {
          $field.trigger('click');
        });
        $field.after($group).hide();
        $group.after($textField);
        $field.bind('change', function () {
            var $input = $(this),
                numFiles = $input.get(0).files ? $input.get(0).files.length : 1,
                label = $input.val().replace(/\\/g, '/').replace(/.*\//, '');
            if (numFiles == 1) {
              $input.next('.input-group-btn').next('input').val(label);
            }
            else if (numFiles > 1) {
              $input.next('.input-group-btn').next('input').val(Drupal.t('@count files selected', {
                '@count': numFiles
              }));
            }
            else {
              $input.next('.input-group-btn').next('input').val('');
            }
        });
      });
    }
  };

})(jQuery);
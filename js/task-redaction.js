(function ($) {

  "use strict";

  Drupal.behaviors.ditoolsiTrainingToggleRedactionForm = {
    attach: function() {
      $('li.redaction-item a').once('ditoolsiTrainingToggleRedactionForm', function () {
        $(this).bind('click', function (e) {
          e.preventDefault();

          $('.redaction-list li').find('a.active').removeClass('active');
          $(this).addClass('active');

          if ($(this).hasClass('current-task')) {
            $('#task-redaction').hide();
            $('#task-answer').show();
          }
          else if ($(this).hasClass('redaction-task')) {
            $('#task-redaction').show();
            $('#task-answer').hide();
          }
        });
      });
    }
  };

  Drupal.behaviors.ditoolsiTrainingCKEditorSetReadonly = {
    attach: function() {
      
      CKEDITOR.on('instanceReady', function (event) {
            var elementId = 'edit-answer-value';

            for (var key in CKEDITOR.instances) {
              var editor = CKEDITOR.instances[ key ];
              if (editor.name !== elementId) {
                editor.setReadOnly(true);
              }
            };
        });
    }
  };

  Drupal.behaviors.ditoolsiTrainingCKEditorSetColor = {
    attach: function(context, setting) {
      $('#task-answer form').once('ditoolsiTrainingCKEditorSetColor', function () {
        var $form = $('#task-answer form');

        var interval = setInterval(function () {
          if ($form.find('.cke_wysiwyg_frame').length) {
            var $iframe = $form.find('.cke_wysiwyg_frame'),
                frameWindow = $iframe.get(0).contentWindow,
                color = '';

            if (setting.ditoolsi.pupil) {
              color = 'blue';
            } else {
              color = 'red';
            }

            var interval2 = setInterval(function () {
              if (frameWindow.document.body) {
                $(frameWindow.document).bind('keydown', function () {
                  frameWindow.document.execCommand('styleWithCSS', false, true);
                  frameWindow.document.execCommand('foreColor', false, color);
                });
                clearInterval(interval2);
              }
            }, 100);

            clearInterval(interval);
          }
        }, 100);
      });
    }
  };

})(jQuery);

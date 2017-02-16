(function ($) {

  "use strict";

  Drupal.behaviors.ditoolsiTrainingAttachContentTabs = {
    attach: function() {
      $('#to-video, #to-audio, #to-presentation, #to-task')
        .find('.ui-slider-tab-content')
        .find('.ajax-new-content')
        .each(function (i, e) {
          if (!$(e).parent().hasClass('.form-group')) {
            $(e).prev().unwrap();
            $(e).remove();
          }
        });

      $('#to-video, #to-audio, #to-presentation, #to-task').once('ditoolsiTrainingAttachContentTabs', function () {
        $(this).sliderTabs({
          panelArrows: false,
          mousewheel: false,
          transitionEasing: 'easeInOutCirc'
        });
      });

      // if ($('#to-video').hasClass('selected')) {
      //   $('#lesson-attach-content').resize($('#to-video').positionPanelArrows);
      // }
    },
    detach: function() {
      $('#to-video, #to-audio, #to-presentation, #to-task')
        .find('.ui-slider-tab-content')
        .find('.ajax-new-content')
        .each(function (i, e) {
          if (!$(e).parent().hasClass('.form-group')) {
            $(e).prev().unwrap();
            $(e).remove();
          }
        });
    }
  };

  Drupal.behaviors.ditoolsiTrainingAttachTabs = {
    attach: function() {
      $('#lesson-attach-content').once('ditoolsiTrainingAttachTabs', function () {
        var slider = $(this).sliderTabs({
          panelArrows: false,
          mousewheel: false,
          transitionEasing: 'easeInOutCirc'
        });

        var $nav = $('#lesson-attach-tabs'),
            $border = $('<div />', {
              id: 'lesson-attach-border'
            }).appendTo($nav);

        function rebuildBorder() {
          var style = '',
              defaultLeft = Math.round($nav.find('li.selected').offset().left - $nav.offset().left),
              defaultTop  = $nav.find('li.selected').height(),
              defaultWidth = $nav.find('li.selected').outerWidth();

          $border.css({
            left: defaultLeft,
            top: defaultTop,
            width: defaultWidth
          });
        }

        $nav.bind('changedTab', rebuildBorder);

        rebuildBorder();
      });
    }
  };

  Drupal.behaviors.ditoolsiTrainingUploadFile = {
    attach: function() {
      $('#add-lesson-form')
        .find('.form-managed-file')
        .once('ditoolsiTrainingUploadFile', function () {
          var $container = $(this),
              id = $container.attr('id'),
              $input  = $('#' + id).find('.form-file'),
              $label  = $('label[for="' + id + '"]'),
              $submit = $(this).find('.form-submit'),
              $add    = $('button.add-button[target="' + id + '"]'),
              selectFile = function() {
                if ($input.length) {
                  $input.trigger('click');
                }
              };

          $label.bind('click', selectFile);

          $add
            .unbind()
            .bind('click', selectFile);

          $input.bind('change', function () {
            $submit.trigger('mousedown');
            $container
              .append('<div class="loading">' + Drupal.t('Loading...') + '</div>');
          });
        });

      var fid, src, $video, $source, $wrapper, $label;

      $('#to-video, #to-audio, #to-presentation, #to-task')
        .find('.form-type-managed-file')
        .once('ditoolsiTrainingUploadFile', function () {
          $(this).each(function (o, e) {
            src = $(e).find('.file a').attr('href');

            if (src) {
              if ($(this).parents('#to-video').length) {
                $wrapper = $('<div />', {
                  class: 'video-element'
                });
                $label = $('<label />', {
                  class: 'label'
                }).html(Drupal.t('Your video') + ':').appendTo($wrapper);
                $video = $('<video />', {
                  controls: 'controls',
                  width: '100%'
                }).appendTo($wrapper);
                $source = $('<source />', {
                  src: src,
                  type: 'video/mp4'
                }).appendTo($video);
                $(this).append($wrapper);
                $video.css({
                  height: ($video.width() * (9 / 15))
                });
              }

              $(e).find('.form-managed-file').addClass('file-uploaded');
              $(this).find('label[for^="upload-lesson-video-file"],\
                label[for^="upload-lesson-audio-file"],\
                label[for^="upload-lesson-presentation-file"],\
                label[for^="upload-lesson-task-file"]').addClass('hide');
              $(this).find('button[target^="upload-lesson-video-file"],\
                button[target^="upload-lesson-audio-file"],\
                button[target^="upload-lesson-presentation-file"],\
                button[target^="upload-lesson-task-file"]').addClass('hide');
              $(this).parents('[id^="to-task-"]').find('.form-type-textfield').hide();
                $(this).parents('[id^="to-video-"]').find('.form-type-textfield[class*="url"]').hide();
            } else {
              $(e).find('.form-managed-file').removeClass('file-uploaded');
              $(this).find('label[for^="upload-lesson-video-file"],\
                label[for^="upload-lesson-audio-file"],\
                label[for^="upload-lesson-presentation-file"],\
                label[for^="upload-lesson-task-file"]').removeClass('hide');
              $(this).find('button[target^="upload-lesson-video-file"],\
                button[target^="upload-lesson-audio-file"],\
                button[target^="upload-lesson-presentation-file"],\
                button[target^="upload-lesson-task-file"]').removeClass('hide');
              $(this).find('video').remove();
              $(this).parents('[id^="to-task-"]').find('.form-type-textfield').show();
                $(this).parents('[id^="to-video-"]').find('.form-type-textfield[class*="url"]').show();
            }
          });
        });
    }
  };

  Drupal.behaviors.ditoolsiTrainingAddYoutubeVideo = {
    attach: function() {
      $('#to-video .form-type-textfield[class*="url"]').once('ditoolsiTrainingAddYoutubeVideo', function () {
        $(this)
          .find('.add-button')
          .bind('click', function () {
            var url = $(this).prev('.form-text').val(),
                regExp = /^.*((youtu.be\/)|(v\/)|(\/u\/\w\/)|(embed\/)|(watch\?))\??v?=?([^#\&\?]*).*/,
                match = url.match(regExp);

            if (match && match[7].length == 11) {
              $(this)
                .parents('.form-group.selected[id^="to-video-"]')
                .find('video')
                .remove();

              var src = 'http://www.youtube.com/embed/' + match[7],
                  $wrapper = $('<div />', {
                    class: 'youtube-iframe'
                  }),
                  $label = $('<span />', {
                    class: 'label'
                  }).html(Drupal.t('Your video') + ':').appendTo($wrapper),
                  $iframe = $('<iframe />', {
                    frameborder: 0,
                    allowfullscreen: 'allowfullscreen',
                    width: '100%'
                  }).appendTo($wrapper);

              $(this)
                .parents('.form-group.selected[id^="to-video-"]')
                .find('.form-type-textfield[class*="url"]')
                .after($wrapper);

              $(this)
                .parents('.form-group.selected[id^="to-video-"]')
                .find('.form-type-managed-file')
                .hide();

              $iframe
                .css({
                  height: ($iframe.width() * (9 / 15))
                })
                .attr('src', src);

              var $fileA = $('<a />', {
                    href: 'https://www.youtube.com/watch?v=' + match[7],
                    target: '_blank'
                  }).html('https://www.youtube.com/watch?v=' + match[7]),
                  $removeBtn = $('<button />', {
                    class: 'remove-btn'
                  }),
                  $file = $('<div />', {
                    class: 'file file-youtube',
                  })
                  .append($fileA)
                  .append($removeBtn);

              $removeBtn.bind('click', function () {
                $(this)
                  .parents('.form-group.selected[id^="to-video-"]')
                  .find('.form-type-managed-file')
                  .show();

                $(this)
                  .parent('.file')
                  .next()
                  .show();

                $(this)
                  .parents('.form-group.selected[id^="to-video-"]')
                  .find('.youtube-iframe')
                  .remove();

                $file.remove();
              });

              $(this)
                .parents('.form-type-textfield')
                .before($file)
                .hide();

              $(this)
                .parents('.form-group.selected[id^="to-video-"]')
                .find('.form-type-managed-file .btn-danger')
                .trigger('mousedown');
            }
          });
      });
    }
  };

  Drupal.behaviors.ditoolsiTrainingLessonSelectList = {
    attach: function() {
      $('#add-lesson-form')
        .find('.task-type-select')
        .once('ditoolsiTrainingLessonSelectList', function () {
          $(this).ksSelecter({

          });
        });
    }
  };

  Drupal.behaviors.ditoolsiTrainingQuestionsMultiple = {
    attach: function() {
      $('#add-lesson-form')
        .find('.add-question-button')
        .once('ditoolsiTrainingQuestionsMultiple', function () {
          $(this).bind('click', function (e) {
            // e.preventDefault();
          });
        });
    }
  };

})(jQuery);

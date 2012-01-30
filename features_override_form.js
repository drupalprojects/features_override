(function ($) {
  Drupal.behaviors.features_override_form = {
    attach: function(context, settings) {
      $('input[type=checkbox][name^="sources[features_override]"]:not(.features-override-form-processed)', context).each(function (i) {
        var $parent_checkbox = $(this);
        var $parent_label = $parent_checkbox.parent().find('label');
        $parent_checkbox.addClass('features-override-form-processed');
        if (this.value.split("__44__").length - 1 == 1) {
          // See if any children.
          var $children = $('input[type=checkbox][name^="sources[features_override]"][value^="' + this.value + '__44__' + '"]', context).parent();
          if ($children.length) {
            $children.wrapAll('<div class="features-override-children-wrapper"></div>');
            $children.find('input').change(function() {
              if ($children.find('input:checked').length) {
                $parent_checkbox.attr('disabled', 'disabled');
                $parent_checkbox.removeAttr('checked');
                if (!$parent_label.parent().find('.description').length) {
                  $parent_label.append('<span class="description"> - ' + Drupal.t('This cannot be picked while individual alters are selected.') + '</span>');
                }
              }
              else {
                $parent_checkbox.removeAttr('disabled');
                $parent_label.parent().find('.description').remove();
              }
            });
            // Hide the elements till someone wants them.
            if (!$children.find('input:checked').length) {
              $children.parents('.features-override-children-wrapper').hide();
              
              $parent_label.append('<a class="features-override-show-children" href="#"> ' + Drupal.t('Select individual overrides') + '</a>');
              $('.features-override-show-children', $parent_label).click(function() {
                $children.parents('.features-override-children-wrapper').show();
                $(this).remove();
                return false;
              });
            }
            else {
              $parent_checkbox.attr('disabled', 'disabled');
              $parent_checkbox.removeAttr('checked');
              if (!$parent_label.parent().find('.description').length) {
                $parent_label.append('<span class="description"> - ' + Drupal.t('This cannot be picked while individual alters are selected.') + '</span>');
              }
            }
          }
          else {
            $parent_checkbox.attr('disabled', 'disabled');
            $parent_label.append('- <span class="description">' + Drupal.t('All alters of this component have been exported to other features.') + '</span>');
          }
        }
      });
    }
  }


})(jQuery);

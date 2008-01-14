/**
 * Modified Star Rating - jQuery plugin
 *
 * Copyright (c) 2006 Wil Stuckey
 *
 * Original source available: http://sandbox.wilstuckey.com/jquery-ratings/
 * Extensively modified by Lullabot: http://www.lullabot.com
 *
 * Dual licensed under the MIT and GPL licenses:
 *   http://www.opensource.org/licenses/mit-license.php
 *   http://www.gnu.org/licenses/gpl.html
 */

/**
 * Create a degradeable star rating interface out of a simple form structure.
 * Returns a modified jQuery object containing the new interface.
 *   
 * @example jQuery('form.rating').rating();
 * @cat plugin
 * @type jQuery 
 *
 */
(function($){ // Create local scope.
    /**
     * Takes the form element, builds the rating interface and attaches the proper events.
     * @param {Object} $obj
     */
    var buildRating = function($obj){
        var $widget = buildInterface($obj),
            $stars = $('.star', $widget),
            $cancel = $('.cancel', $widget),
            $summary = $('.description', $obj).size() ? $('.description', $obj) : $($obj).siblings('.description'),
            summaryText = $summary.html(),
            summaryHover = $obj.is('.fivestar-labels-hover'),
            currentValue = $("select", $obj).val(),
            cancelTitle = $('label', $obj).html(),
            voteTitle = cancelTitle != Drupal.settings.fivestar.titleAverage ? cancelTitle : Drupal.settings.fivestar.titleUser,
            voteChanged = false;

        // Record star display.
        if ($obj.is('.fivestar-user-stars')) {
          var starDisplay = 'user';
        }
        else if ($obj.is('.fivestar-average-stars')) {
          var starDisplay = 'average';
          currentValue = $("input[@name=vote_average]", $obj).val();
        }
        else if ($obj.is('.fivestar-combo-stars')) {
          var starDisplay = 'combo';
        }
        else {
          var starDisplay = 'none';
        }

        // Smart is intentionally separate, so the average will be set if necessary.
        if ($obj.is('.fivestar-smart-stars')) {
          var starDisplay = 'smart';
        }

        // Record text display.
        if ($obj.is('.fivestar-user-text')) {
          var textDisplay = 'user';
        }
        else if ($obj.is('.fivestar-average-text')) {
          var textDisplay = 'average';
        }
        else if ($obj.is('.fivestar-combo-text')) {
          var textDisplay = 'combo';
        }
        else {
          var textDisplay = 'none';
        }

        // Add hover and focus events.
        $stars
            .mouseover(function(){
                event.drain();
                event.fill(this);
            })
            .mouseout(function(){
                event.drain();
                event.reset();
            })
            .focus(function(){
                event.drain();
                event.fill(this)
            })
            .blur(function(){
                event.drain();
                event.reset();
            });
        
        // Cancel button events.
        $cancel
            .mouseover(function(){
                event.drain();
                $(this).addClass('on')
            })
            .mouseout(function(){
                event.reset();
                $(this).removeClass('on')
            })
            .focus(function(){
                event.drain();
                $(this).addClass('on')
            })
            .blur(function(){
                event.reset();
                $(this).removeClass('on')
            });
        
        // Click events.
        $cancel.click(function(){
            currentValue = 0;
            event.reset();
            voteChanged = false;
            // Save the currentValue in a hidden field.
            $("select", $obj).val(0);
            // Update the title.
            cancelTitle = starDisplay != 'smart' ? cancelTitle : Drupal.settings.fivestar.titleAverage;
            $('label', $obj).html(cancelTitle);
            // Submit the form if needed.
            $("input.fivestar-path", $obj).each(function () { $.ajax({ type: 'GET', dataType: 'xml', url: this.value + '/' + 0, success: voteHook }); });
            return false;
        });
        $stars.click(function(){
            currentValue = $('select option', $obj).get($stars.index(this) + $cancel.size() + 1).value;
            // Save the currentValue to the hidden select field.
            $("select", $obj).val(currentValue);
            // Update the display of the stars.
            voteChanged = true;
            event.reset();
            // Submit the form if needed.
            $("input.fivestar-path", $obj).each(function () { $.ajax({ type: 'GET', dataType: 'xml', url: this.value + '/' + currentValue, success: voteHook }); });
            return false;
        });

        var event = {
            fill: function(el){
              // Fill to the current mouse position.
              var index = $stars.index(el) + 1;
              $stars
                .children('a').css('width', '100%').end()
                .filter(':lt(' + index + ')').addClass('hover').end();
              // Update the description text and label.
              if (summaryHover) {
                var summary = $("select option", $obj)[index + $cancel.size()].text;
                var value = $("select option", $obj)[index + $cancel.size()].value;
                $summary.html(summary != index + 1 ? summary : '&nbsp;');
                $('label', $obj).html(voteTitle);
              }
            },
            drain: function() {
              // Drain all the stars.
              $stars
                .filter('.on').removeClass('on').end()
                .filter('.hover').removeClass('hover').end();
              // Update the description text.
              if (summaryHover) {
                var cancelText = $("select option", $obj)[0].text;
                $summary.html(($cancel.size() && cancelText != 0) ? cancelText : '&nbsp');
                if (!voteChanged) {
                  $('label', $obj).html(cancelTitle);
                }
              }
            },
            reset: function(){
              // Reset the stars to the default index.
              var starValue = currentValue/100 * $stars.size();
              var percent = (starValue - Math.floor(starValue)) * 100;
              $stars.filter(':lt(' + Math.floor(starValue) + ')').addClass('on').end();
              if (percent > 0) {
                $stars.eq(Math.floor(starValue)).addClass('on').children('a').css('width', percent + "%").end().end();
              }
              // Restore the summary text and original title.
              if (summaryHover) {
                $summary.html(summaryText ? summaryText : '&nbsp;');
              }
              if (voteChanged) {
                $('label', $obj).html(voteTitle);
              }
              else {
                $('label', $obj).html(cancelTitle);
              }
            }
        };

        /**
         * Checks for the presence of a javascript hook 'fivestarResult' to be
         * called upon completion of a AJAX vote request.
         */
        var voteHook = function(data) {
          var returnObj = {
            result: {
              count: $("result > count", data).text(),
              average: $("result > average", data).text(),
              summary: { average: $("summary average", data).text(), user: $("summary user", data).text(), combo: $("summary combo", data).text() }
            },
            vote: {
              id: $("vote id", data).text(),
              type: $("vote type", data).text(),
              value: $("vote value", data).text()
            },
            display: {
              stars: starDisplay,
              text: textDisplay
            }
          };
          // Check for a custom callback.
          if (window.fivestarResult) {
            fivestarResult(returnObj);
          }
          // Use the default.
          else {
            fivestarDefaultResult(returnObj);
          }
          // Update the summary text.
          summaryText = returnObj.result.summary[returnObj.display.text];
          // Update the current star currentValue to the previous average.
          if (returnObj.vote.value == 0 && starDisplay == 'average') {
            currentValue = returnObj.result.average;
            event.reset();
          }
        };

        event.reset();
        return $widget;
    };
    
    /**
     * Accepts jQuery object containing a single fivestar widget.
     * Returns the proper div structure for the star interface.
     * 
     * @return jQuery
     * @param {Object} $widget
     * 
     */
    var buildInterface = function($widget){
        var $container = $('<div class="fivestar-widget clear-block"></div>');
        var $options = $("select option", $widget);
        var size = $('option', $widget).size() - 1;
        var cancel = 1;
        for (var i = 1, option; option = $options[i]; i++){
            if (option.value == "0") {
              cancel = 0;
              $div = $('<div class="cancel"><a href="#0" title="' + option.text + '">' + option.text + '</a></div>');
            }
            else {
              var zebra = (i + cancel - 1) % 2 == 0 ? 'even' : 'odd';
              var count = i + cancel - 1;
              var first = count == 1 ? ' star-first' : '';
              var last = count == size + cancel - 1 ? ' star-last' : '';
              $div = $('<div class="star star-' + count + ' star-' + zebra + first + last + '"><a href="#' + option.value + '" title="' + option.text + '">' + option.text + '</a></div>');
            }
            $container.append($div[0]);                    
        }
        $container.addClass('fivestar-widget-' + (size + cancel - 1));
        // Attach the new widget and hide the existing widget.
        $('select', $widget).after($container).hide();
        return $container;
    };

    /**
     * Standard handler to update the average rating when a user changes their
     * vote. This behavior can be overridden by implementing a fivestarResult
     * function in your own module or theme.
     * @param object voteResult
     * Object containing the following properties from the vote result:
     * voteResult.result.count The current number of votes for this item.
     * voteResult.result.average The current average of all votes for this item.
     * voteResult.result.summary.average The textual description of the average.
     * voteResult.result.summary.user The textual description of the user's current vote.
     * voteResult.vote.id The id of the item the vote was placed on (such as the nid)
     * voteResult.vote.type The type of the item the vote was placed on (such as 'node')
     * voteResult.vote.average The average of the new vote saved
     * voteResult.display.stars The type of star display we're using. Either 'average', 'user', or 'combo'.
     * voteResult.display.text The type of text display we're using. Either 'average', 'user', or 'combo'.
     */
    function fivestarDefaultResult(voteResult) {
      // Update the summary text.
      $('div.fivestar-summary-'+voteResult.vote.id).html(voteResult.result.summary[voteResult.display.text]);
      // If this is a combo display, update the average star display.
      if (voteResult.display.stars == 'combo') {
        $('div.fivestar-form-'+voteResult.vote.id).each(function() {
          var $stars = $('.fivestar-widget-static .star span', this);
          var average = voteResult.result.average/100 * $stars.size();
          var index = Math.floor(average);
          $stars.removeClass('on').addClass('off').css('width', 'auto');
          $stars.filter(':lt(' + (index + 1) + ')').removeClass('off').addClass('on');
          $stars.eq(index).css('width', ((average - index) * 100) + "%");
        });
      }
    };

    /**
     * Set up the plugin
     */
    $.fn.rating = function() {
      var stack = [];
      this.each(function() {
          var ret = buildRating($(this));
          stack.push(ret);
      });
      return stack;
    };

  // Fix ie6 background flicker problem.
  if ($.browser.msie == true) {
    try {
      document.execCommand('BackgroundImageCache', false, true);
    } catch(err) {}
  }
})(jQuery);

if (Drupal.jsEnabled) {
  $(document).ready(function() {
    $('div.fivestar-form-item').rating();
    $('input.fivestar-submit').hide();
  });
}
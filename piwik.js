(function ($) {

$(document).ready(function() {

  // Colorbox: This event triggers when the transition has completed and the
  // newly loaded content has been revealed.
  $(document).bind("cbox_complete", function () {
    var href = $.colorbox.element().attr("href");
    if (href) {
      _paq.push(["setCustomUrl", href]);
      _paq.push(["trackPageView"]);
    }
  });

});


})(jQuery);

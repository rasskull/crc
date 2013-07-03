(function ($) {
  $(document).ready(function(){
  
    //vbs
    $('body.page-node-add-vbs-child h1.title').text('Register a child for VBS');
  
    // add one of these lines for every year past so people cannot modify it
    $("#edit-field-child-vbssession-und-2010-summer").attr('disabled', 'disabled');
    $(".form-item-field-child-vbssession-und-2010-Summer label").addClass('past-vbs').html('<em>2010 Summer - this session has passed</em>');
    
    // 2011
    $("#edit-field-child-vbssession-und-2011-summer").attr('disabled', 'disabled');
    $(".form-item-field-child-vbssession-und-2011-Summer label").addClass('past-vbs').html('<em>2011 Summer - this session has passed</em>');
    
    // 2012
    $("#edit-field-child-vbssession-und-2012-summer").attr('disabled', 'disabled');
    $(".form-item-field-child-vbssession-und-2012-Summer label").addClass('past-vbs').html('<em>2012 Summer - this session has passed</em>');
    

    $('.field-name-field-message-audio').after("<div class='message-sub-text'>Click the play button to listen to the message or right click on the file name under the player and choose 'save target as' or 'download linked file as' to save it to your computer)</div>");
     
  });
})(jQuery); 
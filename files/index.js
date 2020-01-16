function get_config() {
	$.getJSON('/config',
      function(data) {
          $.each(data, function(key, value){
             document.getElementById(key).value = value;
      });
    });
}
function post_netcfg(event) {
	event.preventDefault();
	var formData = {
			'tennentid'				:	document.getElementById('tennentid').value,
			'client_id'			:	document.getElementById('client_id').value,
			};
	$.ajax({
        type        : 'POST',
        url         : '/config',
        contentType	: 'application/json; charset=utf-8',
        data        : JSON.stringify(formData),
        dataType	: 'json'
    })
}
$( document ).ready(function() {
	get_config();
	
	document.getElementById('form_netcfg').addEventListener('submit', post_netcfg);
	document.getElementById('netcfg_cancel').addEventListener('click', get_config);
});

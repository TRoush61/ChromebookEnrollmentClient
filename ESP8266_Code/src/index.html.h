const char INDEX_HTML[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Chromebook Enrollment Client</title>
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.0/css/bootstrap.min.css">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.0/js/bootstrap.min.js"></script>
  </head>
  <body>
    <div class="container">
      <nav class="navbar navbar-inverse">
        <div class="container-fluid">
            <div class="navbar-header">
            <a class="navbar-brand" href="#">Chromebook Enrollment Client</a>
            </div>
            <ul class="nav navbar-nav">
            <li class="active"><a href="#">Home</a></li>
            <li><a href="update">Firmware Update</a></li>
            </ul>
        </div>
      </nav>
      <div class="container-fluid">
        <div class="jumbotron">
            <img src="https://lh3.googleusercontent.com/uMVyiVMEKr9SYnY8g90tGVwKAuAAZrC2urEVASlb9G89PBhEqeuE1PD01PWppV_RbZIr=w1000" width="500px" height="500px"
            style="display: block;
            margin-left: auto;
            margin-right: auto;
            width: 50%;"/>
            <h2>Welcome to the Chromebook Enrollment Client</h2>
            <p>Welcome to the Chromebook Enrollment Client. Your client is up and running perfectly! If you need to update the firmware, you can do so at <a href="update">the update page.</a></a></p>
        </div>
      </div>
    </div>
  </body>
</html>
)=====";
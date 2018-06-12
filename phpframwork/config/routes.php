<?php

use NoahBuscher\Macaw\Macaw;

Macaw::get('home', 'App\controllers\HomeController@test');

Macaw::get('(:all)', function (){
   echo 'no page';
});

Macaw::dispatch();
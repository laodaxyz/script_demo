<?php
class IndexController extends Yaf_Controller_Abstract {
   // default action name
   public function indexAction() {  
   		//调用model
   		// $user = new UserModel();
   		// $user->add();

   		$mod = new UserModel();
        $data = $mod->find(1)->toArray();
        print_r($data);

   		//调用library
   		$tool = new Tools();
   		$tool->init();

        $this->getView()->content = "Hello World";
   }
}
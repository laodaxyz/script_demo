<?php

/**
 * 所有在Bootstrap类中, 以_init开头的方法, 都会被Yaf调用,
 * 这些方法, 都接受一个参数:Yaf_Dispatcher $dispatcher
 * 调用的次序, 和申明的次序相同
 */

//composer 加载第三方库
Yaf_loader::import("/vendor/autoload.php");
use Illuminate\Database\Capsule\Manager;

class Bootstrap extends Yaf_Bootstrap_Abstract{

        public function _initConfig() {
            date_default_timezone_set('Asia/Shanghai');
            $this->config = Yaf_Application::app()->getConfig();
            Yaf_Registry::set("config", $this->config);
        }

        public function _initPlugin(Yaf_Dispatcher $dispatcher) {
            $user = new UserPlugin();
            $dispatcher->registerPlugin($user);
        }

        public function _initDefaultName(Yaf_Dispatcher $dispatcher) {
                $dispatcher->setDefaultModule("Index")->setDefaultController("Index")->setDefaultAction("index");
        }

        //载入数据库ORM
	    public function _initDatabase(){
	        $database = array(
	            'driver'    => $this->config->db->type,
	            'host'      => $this->config->db->host,
	            'database'  => $this->config->db->database,
	            'username'  => $this->config->db->username,
	            'password'  => $this->config->db->password,
	            'charset'   => $this->config->db->charset,
	            'collation' => $this->config->db->collation,
	            'prefix'    => $this->config->db->prefix,
	        );
	        $manager = new Manager;
	        // 创建链接
            $manager->addConnection($database);
	        // 设置全局静态可访问
            $manager->setAsGlobal();
	        // 启动Eloquent
            $manager->bootEloquent();
	    }
}
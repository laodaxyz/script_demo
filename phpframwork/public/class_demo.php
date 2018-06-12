<?php
namespace MyProject1;

class BaseClass {

    public $a        = 'base_a';   //访问控制  public protected private
    public static $b = 'base_static_b';
    const NAME       = 'BASE';

    function __construct() {
        $this->saya();
        self::sayb();
        echo "BaseClass 类中构造方法";
    }

    public function saya(){
        echo 'say=>  '.$this->a.self::NAME;
    }

    public static function sayb(){
        echo 'say=>  '.self::$b.self::NAME;
    }

    function __destruct() {
        print "销毁BaseClass";
    }
}

namespace MyProject2;
use MyProject1 as base;

class SubClass extends base\BaseClass {

    public $a        = 'sub_a';
    public static $b = 'sub_static_b';
    const NAME       = 'SUB';

    function __construct() {
        parent::__construct();  // 子类构造方法不能自动调用父类的构造方法
        echo "SubClass 类中构造方法";
    }

    public function saya(){
        parent::saya();
    }
}


// 调用 BaseClass 构造方法
$obj = new base\BaseClass();
echo "<br>";
$obj->saya();
echo "<br>";
$obj::sayb();
echo "<br>";

// 调用 BaseClass、SubClass 构造方法
$obj = new SubClass();
echo "<br>";
$obj->saya();
echo "<br>";
$obj::sayb();
echo "<br>";










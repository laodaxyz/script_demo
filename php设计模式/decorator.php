<?php

interface Decotator{
    public function display();
}

class XiaoFang implements Decotator
{
    private $name;

    public function __construct($name){
        $this->name = $name;
    }

    public function display(){
        echo 'my name is '.$this->name.'<br>';
    }
}
class Finery implements Decotator
{
    private $component;

    public function __construct(Decotator $component){
        $this->component = $component;
    }

    public function display(){
        $this->component->display();
    }
}
class Shoes extends Finery{
    public function display(){
        echo 'shoes'.'<br>';
        parent::display();
    }
}
class Shirt extends Finery{
    public function display()
    {
        echo 'Shirt'.'<br>';
        parent::display();
    }
}
class Fire extends Finery{
    public function display()
    {
        echo 'Fire befer'.'<br>';
        parent::display();
        echo 'Fire after'.'<br>';
    }
}
$xiaofang = new XiaoFang('xiaofang');
$shoes = new Shoes($xiaofang);
$skirt = new Shirt($shoes);
$fire = new Fire($skirt);
$fire->display();
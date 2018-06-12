<?php
use Illuminate\Database\Eloquent\Model as Mymodel;

class UserModel extends Mymodel{

	protected $table = 'users';

    public function add() {
            echo "model_add";
    }
}
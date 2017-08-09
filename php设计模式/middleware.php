<?php

interface Middleware{
    public static function handle(Closure $next);
}

class VerifyCsrfToken implements Middleware{
    public static function handle(Closure $next){
        echo '验证VerifyCsrfToken'.'<br>';
        $next();
    }
}
class ShareErrorsFromSession implements Middleware{
    public static function handle(Closure $next){
        echo '共享ShareErrorsFromSession'.'<br>';
        $next();
    }
}
class StartSession implements Middleware{
    public static function handle(Closure $next){
        echo '+++start StartSession'.'<br>';
        $next();
        echo '+++end   StartSession'.'<br>';
    }
}
class AddQueue implements Middleware{
    public static function handle(Closure $next){
        $next();
        echo '添加队列AddQueue'.'<br>';
    }
}
function getSlice(){
    return function ($stack, $pipe){
        return function () use ($stack, $pipe){
            return $pipe::handle($stack);
        };
    };
}
function then(){
    $pipes = [
        'AddQueue',
        'StartSession',
        'ShareErrorsFromSession',
        'VerifyCsrfToken',
    ];
    $firstSlice = function (){
        echo '请求想路由器传递，返回响应'.'<br>';
    };
    $pipes = array_reverse($pipes);
    call_user_func(
        array_reduce($pipes, getSlice(), $firstSlice)
    );
}
then();
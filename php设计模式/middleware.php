<?php

interface Middleware{
    public static function handle(Closure $next);
}

class VerifyCsrfToken implements Middleware{
    public static function handle(Closure $next){
        echo '3=====验证csrf令牌====='.'<br>';
        $next();
        echo '3=====添加csrf令牌====='.'<br>';
    }
}
class ShareErrorsFromSession implements Middleware{
    public static function handle(Closure $next){
        echo '如果session中有error变量，则共享它'.'<br>';
        $next();
    }
}
class StartSession implements Middleware{
    public static function handle(Closure $next){
        echo '2+++start 开启session'.'<br>';
        $next();
        echo '2+++end   关闭session，存储会话数据'.'<br>';
    }
}
class AddQueuedCookieToResponse implements Middleware{
    public static function handle(Closure $next){
        $next();
        echo '添加队列cookie到响应'.'<br>';
    }
}
class EncryptCookies implements Middleware{
    public static function handle(Closure $next){
        echo '1===cookie解密处理==='.'<br>';
        $next();
        echo '1===cookie加密处理==='.'<br>';
    }
}
class CheckForMaintenanceMode implements Middleware{
    public static function handle(Closure $next){
        echo '确定当前程序是否处于维护模式'.'<br>';
        $next();
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
        'CheckForMaintenanceMode',
        'EncryptCookies',
        'AddQueuedCookieToResponse',
        'StartSession',
        'ShareErrorsFromSession',
        'VerifyCsrfToken',
    ];
    $firstSlice = function (){
        echo '[[[  请求想路由器传递，返回响应  ]]]'.'<br>';
    };
    $pipes = array_reverse($pipes);
    $call = array_reduce($pipes, getSlice(), $firstSlice);

    //$call();
    call_user_func($call);
}
then();
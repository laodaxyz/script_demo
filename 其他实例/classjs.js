/*
* ��ʽ�̳У����캯����
*/
var father = function(){
	//this.name = 'wangwu';
	this.age = 52;
	this.say = function(){
		alert('hello i am '+this.name+' and i am '+this.age+' years old');
	}
}
var child = function(){
	this.name = 'lisi';
	father.call(this);
}
var man = new child();
man.say();

/*
* ԭ�ͼ̳� 
*/
var father = function(){}
father.prototype.a = function(){ }
var child = function(){}
//��ʼ�̳�
child.prototype = new father();
var man = new child();
man.a();

/*
* ��ϼ̳�
*/
function father(){
	this.a = 'father'
}
father.prototype.b = function(){
	alert(this.a);
}
var child = function(){
	father.call(this);
}
child.prototype = new father();

/*
* new �̳�
*/
var father = function(){
	this.a = 'father';
}
father.prototype.b = function(){
	alert(this.a)
}
var obj = new father()
/*
* object.create ��ʽ�̳�
*/
var father = {
	a:'father',
	b:function(){
		alert(this.a)
	}
}
var obj = Object.create(father);
console.dir(obj)
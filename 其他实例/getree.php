<?php
$datas = Array ( Array (  'ParentCode'=> 1, 'Code' => 8, 'Name'=>'总裁办' ), 
				Array (  'ParentCode'=> 1,'Code'=> 32,'Name'=> '运营平台' ),
				Array ( 'ParentCode' => 1 ,'Code' => 37 ,'Name' => '运营平台' ),
				Array ( 'ParentCode' => 32 ,'Code' => 6 ,'Name' => '巨龙工作室' ,'ParentName' => '运营平台' ),
				Array ( 'ParentCode' => 32 ,'Code' => 7 ,'Name' => '水世界工作室' ,'ParentName' => '运营平台' ),
				Array ( 'ParentCode' => 32 ,'Code' => 33 ,'Name' => '格美时空工作室' ,'ParentName' => '运营平台' ),
				Array ( 'ParentCode' => 32 ,'Code' => 34 ,'Name' => '魔幻动力深圳工作室' ,'ParentName' => '运营平台' ),
				Array ( 'ParentCode' => 37 ,'Code' => 2 ,'Name' => '综合管理中心' ,'ParentName' => '运营平台' )
);

//通用树
function getTree($data, $pId=1,$level=1,$level_name=''){
	foreach($data as $k => $v){
		if($v['ParentCode'] == $pId){
			$v['level'] = $level+1;
			$v['level_name'] = $level_name.'-'.$v['Name'];
			$v['Childs'] = getTree($data, $v['Code'],$level,$v['Name']);
			$tree[] = $v;
	   }
	}
	$tree = count($tree)>0?$tree:'';
	return $tree;
}
//html遍历输出
function procHtml($tree){
	$html = '';
	foreach($tree as $t){
		if($t['cate_ParentId'] == ''){
			$html .= "<li>{$t['cate_Name']}</li>";
		}else{
			$html .= "<li>".$t['cate_Name'];
			$html .= procHtml($t['cate_ParentId']);
			$html = $html."</li>";
		}
	}
	return $html ? '<ul>'.$html.'</ul>' : $html ;
}

$result = getTree($datas);
print_r($result);

//数据健与ocde值一致
function genTree($items) {
    foreach ($items as $k => $v) {
        $result[$v['Code']] = $v;
    }
    foreach ($result as $item){
        $result[$item['ParentCode']]['son'][$item['Code']] = &$result[$item['Code']];
    }
    return isset($result[0]['son']) ? $result[0]['son'] : array(); 
}

<?php
$datas = Array ( Array (  'ParentCode'=> 1, 'Code' => 8, 'Name'=>'�ܲð�' ), 
				Array (  'ParentCode'=> 1,'Code'=> 32,'Name'=> '��Ӫƽ̨' ),
				Array ( 'ParentCode' => 1 ,'Code' => 37 ,'Name' => '��Ӫƽ̨' ),
				Array ( 'ParentCode' => 32 ,'Code' => 6 ,'Name' => '����������' ,'ParentName' => '��Ӫƽ̨' ),
				Array ( 'ParentCode' => 32 ,'Code' => 7 ,'Name' => 'ˮ���繤����' ,'ParentName' => '��Ӫƽ̨' ),
				Array ( 'ParentCode' => 32 ,'Code' => 33 ,'Name' => '����ʱ�չ�����' ,'ParentName' => '��Ӫƽ̨' ),
				Array ( 'ParentCode' => 32 ,'Code' => 34 ,'Name' => 'ħ�ö������ڹ�����' ,'ParentName' => '��Ӫƽ̨' ),
				Array ( 'ParentCode' => 37 ,'Code' => 2 ,'Name' => '�ۺϹ�������' ,'ParentName' => '��Ӫƽ̨' )
);

//ͨ����
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
//html�������
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

//���ݽ���ocdeֵһ��
function genTree($items) {
    foreach ($items as $k => $v) {
        $result[$v['Code']] = $v;
    }
    foreach ($result as $item){
        $result[$item['ParentCode']]['son'][$item['Code']] = &$result[$item['Code']];
    }
    return isset($result[0]['son']) ? $result[0]['son'] : array(); 
}

<?php 

function ihg_connector_sync(){
	
	$out = new stdClass();
	if(!isset($_GET['v'])){
		$out->status = -1;
		$out->message = "specificare versione";
	}else{
		$v = $_GET['v'];
		$c_v = variable_get('data_last_version', 0);
		if($v < $c_v){
			$out = new stdClass();
			$out->status = 1;
			$out->version = variable_get('data_last_version', 0);
			$out->data = ihg_connector_data_json();
			$out->cities = ihg_connector_cities_json();
		}else{
			$out->status = 0;
			$out->message = "nothing to update";
		}
	}
	drupal_json_output($out);
}

function ihg_connector_images_json(){
	$result = db_query("select uri, timestamp from file_managed")->fetchAll();
	return $result;
}

function ihg_connector_data_json(){
	$hotels = array();
	
	$result = db_query("select node.nid,node.language,node.title, 
			ordinamento.field_ordinamento_value as ordinamento,
			indirizzo.field_indirizzo_value as indirizzo,
			telefono.field_telefono_value as telefono,
			fax.field_fax_value as fax,
			descrizione_welcome.field_descrizione_welcome_value as descrizione_welcome
			from node 
			left join field_data_field_ordinamento ordinamento on ordinamento.entity_id=node.nid
			left join field_data_field_indirizzo indirizzo on indirizzo.entity_id=node.nid
			left join field_data_field_telefono telefono on telefono.entity_id=node.nid
			left join field_data_field_fax fax on fax.entity_id=node.nid
			left join field_data_field_descrizione_welcome descrizione_welcome on descrizione_welcome.entity_id=node.nid
			where type in ('hotel')");
	foreach($result as $record){
		$hotel = new stdClass();
		$hotel->nid = $record->nid;
		$hotel->lingua = $record->language;
		$hotel->ordinamento = $record->ordinamento;
		$hotel->tipo = _fetch_term($record->nid, 'field_tipologia');
		$hotel->nome = $record->title;
		$hotel->indirizzo = $record->indirizzo;
		$hotel->luogo = _fetch_term($record->nid, 'field_localit_');
		$hotel->mappa = _fetch_uri($record->nid, 'field_mappa_struttura');
		$hotel->banner_citta = _fetch_uri($record->nid, 'field_immagine_citt_');
		$hotel->mappa_citta = _fetch_uri($record->nid, 'field_mappa_citt_');
		
		$welcome = new stdClass();
		$welcome->desc = $record->descrizione_welcome;
		$welcome->caratt = _fetch_multiple_value($record->nid, 'field_caratteristica_1_welcome', 'caratteristica');
		$welcome->foto = _fetch_multiple_uri($record->nid, 'field_foto_welcome', 'foto');
		
		
		$camere = new stdClass();
		$camere->caratt = _fetch_multiple_value($record->nid, 'field_caratteristica_1_camere', 'caratteristica');
		$camere->foto = _fetch_multiple_uri($record->nid, 'field_foto_camere', 'foto');
		
		$meetings = new stdClass();
		$meetings->caratt = _fetch_multiple_value($record->nid, 'field_caratteristiche_meetings_e', 'caratteristica');
		$meetings->foto = _fetch_multiple_uri($record->nid, 'field_foto_meetings_eventi', 'foto');

		$servizi = new stdClass();
		$servizi->caratt = _fetch_multiple_value($record->nid, 'field_caratteristiche_servizi', 'caratteristica');
		$servizi->foto = _fetch_multiple_uri($record->nid, 'field_foto_servizi', 'foto');
		
		$contatti = array();
		$contatti[] = array('contatto' => _fetch_contatto_a($record->nid));
		$contatti[] = array('contatto' =>_fetch_contatto_b($record->nid));
		$contatti[] = array('contatto' =>_fetch_contatto_c($record->nid));
		
		$hotel->welcome = $welcome;
		$hotel->meetings = $meetings;
		$hotel->servizi = $servizi;
		$hotel->contatti = $contatti;
		
		$hotel->panormaiche = _fetch_multiple_uri($record->nid, 'field_foto_360', 'foto');
		
		$hotels[] = $hotel;
	}
	return $hotels;
}

function _fetch_term($nid, $field){
	$table = 'field_data_' . $field;
	$column = $field . '_tid';
	$term =  db_query("select term.name from taxonomy_term_data term
join $table t on t.$column=term.tid
where t.entity_id=:nid", array(':nid' => $nid))->fetch();
	return $term->name;
}

function _fetch_uri($nid, $field){
	$table = 'field_data_' . $field;
	$column = $field . '_fid';
	$file = db_query("select uri from file_managed
join $table t on t.$column=file_managed.fid
where t.entity_id=:nid", array(':nid' => $nid))->fetch();
	return $file->uri;
}

function _fetch_multiple_value($nid, $field, $alias){
	$table = 'field_data_' . $field;
	$column = $field . '_value';
	return db_query("select t.$column as $alias from $table t
where t.entity_id=:nid", array(':nid' => $nid))->fetchAll();
}

function _fetch_multiple_uri($nid, $field){
	$table = 'field_data_' . $field;
	$column = $field . '_fid';
	$files = db_query("select uri from file_managed
			join $table t on t.$column=file_managed.fid
			where t.entity_id=:nid", array(':nid' => $nid))->fetchAll();
	return $files;
}

function _fetch_contatto_a($nid){
	return db_query("select 
	field_data_field_mansione.field_mansione_value as mansione,
	field_data_field_nome_e_cognome.field_nome_e_cognome_value as nome,
	field_data_field_email.field_email_value as email,
	file_managed.uri as 'foto'
from field_data_field_mansione
join field_data_field_nome_e_cognome on field_data_field_nome_e_cognome.entity_id=field_data_field_mansione.entity_id
join field_data_field_email on field_data_field_email.entity_id=field_data_field_mansione.entity_id
left join field_data_field_fototessera_a on field_data_field_fototessera_a.entity_id=field_data_field_mansione.entity_id
left join file_managed on file_managed.fid=field_data_field_fototessera_a.field_fototessera_a_fid
where field_data_field_mansione.entity_id=:nid", array(':nid' => $nid))->fetch();
}
function _fetch_contatto_b($nid){
	return db_query("select
	field_data_field_mansioneb.field_mansioneb_value as mansione,
	field_data_field_nome_e_cognomeb.field_nome_e_cognomeb_value as nome,
	field_data_field_emailb.field_emailb_value as email,
	file_managed.uri as 'foto'
from field_data_field_mansioneb
join field_data_field_nome_e_cognomeb on field_data_field_nome_e_cognomeb.entity_id=field_data_field_mansioneb.entity_id
join field_data_field_emailb on field_data_field_emailb.entity_id=field_data_field_mansioneb.entity_id
left join field_data_field_fototessera_b on field_data_field_fototessera_b.entity_id=field_data_field_mansioneb.entity_id
left join file_managed on file_managed.fid=field_data_field_fototessera_b.field_fototessera_b_fid
where field_data_field_mansioneb.entity_id=:nid", array(':nid' => $nid))->fetch();
}
function _fetch_contatto_c($nid){
	return db_query("select
	field_data_field_mansionec.field_mansionec_value as mansione,
	field_data_field_nome_e_cognomec.field_nome_e_cognomec_value as nome,
	field_data_field_emailc.field_emailc_value as email,
	file_managed.uri as 'foto'
from field_data_field_mansionec
join field_data_field_nome_e_cognomec on field_data_field_nome_e_cognomec.entity_id=field_data_field_mansionec.entity_id
join field_data_field_emailc on field_data_field_emailc.entity_id=field_data_field_mansionec.entity_id
left join field_data_field_fototessera_c on field_data_field_fototessera_c.entity_id=field_data_field_mansionec.entity_id
left join file_managed on file_managed.fid=field_data_field_fototessera_c.field_fototessera_c_fid
where field_data_field_mansionec.entity_id=:nid", array(':nid' => $nid))->fetch();
}

function ihg_connector_cities_json(){
	$images = array();
	$banners = db_query("select term.name,f.uri from field_data_field_immagine_citt_ ic
join taxonomy_term_data term on term.tid=ic.entity_id
join file_managed f on f.fid=ic.field_immagine_citt__fid
where ic.bundle=:bundle", array(':bundle' => "localit_sulla_mappa"))->fetchAllAssoc('name');
	$maps = db_query("select term.name,f.uri from field_data_field_mappa_citt_ mappa
join taxonomy_term_data term on term.tid=mappa.entity_id
join file_managed f on f.fid=mappa.field_mappa_citt__fid
where mappa.bundle=:bundle", array(':bundle' => "localit_sulla_mappa"))->fetchAllAssoc('name');
	foreach($banners as $key => $value){
		$images[] = (object)array('name' => $key, 'banner' => $value->uri, 'mappa' => $maps[$key]->uri);
	}
	return $images;
}

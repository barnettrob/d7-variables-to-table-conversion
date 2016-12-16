<?php

/**
 * @file
 * Transfer data related to class_participation from variable table to class_participation_survey.
 */

/**
 * class_participation_survey fields:
 * sid: survey id: unique auto-incrementing
 * type: individual = 0; learning group = 1
 * gid: course id (nid)
 * version: version number for the course.
 * title
 * date (unix timestamp)
 * min
 * initial
 * max
 */

function class_participation_get_variable_data() {
  $cp_survey_sql = db_query("SELECT name, value
                             FROM {variable}
                             WHERE name LIKE 'class_participation_%'
                             AND name NOT LIKE 'class_participation_max_unique_survey_id_%'");

  $new_survey_records = array();

  // Loop through to get the gids for the survey records in the variable table.
  foreach ($cp_survey_sql as $cp_survey_results) {
    $cp_survey_array = explode('_', $cp_survey_results->name);
    // Remove version number off variable name end of array.
    array_pop($cp_survey_array);
    $gids[] = array_pop($cp_survey_array);
  }

  // Only show one gid (nid) to loop through.
  $gids = array_unique($gids);

  $content = '';
  foreach ($gids as $gid) {
    // Get all class participation records based on gid query find.
    $variables = db_query("SELECT name, value
                                  FROM {variable}
                                  WHERE name LIKE :gid", array(':gid' => '%' . db_like($gid) . '%'));
    // Define array to store all class participation variable records in a better way.
    // This will all the records to migrate more easily to a custom class_participation_survey table.
    // Start building out array with gid as the key.
    $new_survey_records[$gid] = array();
    foreach ($variables as $variable) {
        $variable_row_array = explode('_', $variable->name);
        // Get version off end of variable name.
        $version = array_pop($variable_row_array);
        // Get rid of gid from name row.
        array_pop($variable_row_array);
        $variable_row_field = implode('_', $variable_row_array);
        // Keep building out array with all of the data for each survey.
        switch ($variable_row_field) {
          case 'class_participation_date':
            $new_survey_records[$gid][$version]['date'] = unserialize($variable->value);
            break;
          case 'class_participation_title_value':
            $new_survey_records[$gid][$version]['title'] = unserialize($variable->value);
            break;
          case 'class_participation_max_value':
            $new_survey_records[$gid][$version]['max'] = unserialize($variable->value);
            break;
          case 'class_participation_min_value':
            $new_survey_records[$gid][$version]['min'] = unserialize($variable->value);
            break;
          case 'class_participation_starting_value':
            $new_survey_records[$gid][$version]['initial'] = unserialize($variable->value);
            break;
          case 'class_participation_survey_type':
            $new_survey_records[$gid][$version]['type'] = unserialize($variable->value);
            break;
        }
    }
  }

  $content .= '<table border="1">';
  $content .= '<thead>';
  $content .= '<tr>';
  $content .= '<td>';
  $content .= 'gid';
  $content .= '</td>';
  $content .= '<td>';
  $content .= 'version';
  $content .= '</td>';
  $content .= '<td>';
  $content .= 'type';
  $content .= '</td>';
  $content .= '<td>';
  $content .= 'title';
  $content .= '</td>';
  $content .= '<td>';
  $content .= 'date';
  $content .= '</td>';
  $content .= '<td>';
  $content .= 'min';
  $content .= '</td>';
  $content .= '<td>';
  $content .= 'initial';
  $content .= '</td>';
  $content .= '<td>';
  $content .= 'max';
  $content .= '</td>';
  $content .= '</tr>';
  $content .= '</thead>';
  $content .= '<tbody>';

  foreach ($new_survey_records as $gid => $version) {
    foreach ($version as $v => $each_survey) {

      // Type human-translation:
      switch ($each_survey['type']) {
        case 0:
          $type = 'Ind';
          break;
        case 1:
          $type = 'LG';
          break;
        default:
          $type = '';
      }

      $content .= '<tr>';
      $content .= '<td>';
      $content .= $gid;
      $content .= '</td>';
      $content .= '<td>';
      $content .= $v;
      $content .= '</td>';
      $content .= '<td>';
      $content .= $type;
      $content .= '</td>';
      $content .= '<td>';
      $content .= $each_survey['title'];
      $content .= '</td>';
      $content .= '<td>';
      $content .= date('n/d/Y', $each_survey['date']);
      $content .= '</td>';
      $content .= '<td>';
      $content .= $each_survey['min'];
      $content .= '</td>';
      $content .= '<td>';
      $content .= $each_survey['initial'];
      $content .= '</td>';
      $content .= '<td>';
      $content .= $each_survey['max'];
      $content .= '</td>';
      $content .= '</tr>';
    }
  }

  $content .= '</tbody>';
  $content .= '</table>';

  return $content;
}

/**
 * Class Participation variable table data migration to class_participation_survey table.
 */
function class_participation_migrate_variable_data() {
  $migration_status = '';
  // Check that class_participation_survey exists and is empty before migration.
  if (db_table_exists('class_participation_survey')) {
    $cp_survey_table_count_results = db_select('class_participation_survey', 'cps')
      ->fields('cps')
      ->execute();
    $cp_survey_table_count = $cp_survey_table_count_results->rowCount();

    if ($cp_survey_table_count == 0) {
      $cp_survey_sql = db_query("SELECT name, value
                             FROM {variable}
                             WHERE name LIKE 'class_participation_%'
                             AND name NOT LIKE 'class_participation_max_unique_survey_id_%'");

      $new_survey_records = array();

      // Loop through to get the gids for the survey records in the variable table.
      foreach ($cp_survey_sql as $cp_survey_results) {
        $cp_survey_array = explode('_', $cp_survey_results->name);
        // Remove version number off variable name end of array.
        array_pop($cp_survey_array);
        $gids[] = array_pop($cp_survey_array);
      }

      // Only show one gid (nid) to loop through.
      $gids = array_unique($gids);

      foreach ($gids as $gid) {
        // Get all class participation records based on gid query find.
        $variables = db_query("SELECT name, value
                                  FROM {variable}
                                  WHERE name LIKE :gid", array(':gid' => '%' . db_like($gid) . '%'));
        // Define array to store all class participation variable records in a better way.
        // This will all the records to migrate more easily to a custom class_participation_survey table.
        // Start building out array with gid as the key.
        $new_survey_records[$gid] = array();
        foreach ($variables as $variable) {
          $variable_row_array = explode('_', $variable->name);
          // Get version off end of variable name.
          $version = array_pop($variable_row_array);
          // Get rid of gid from name row.
          array_pop($variable_row_array);
          $variable_row_field = implode('_', $variable_row_array);
          // Keep building out array with all of the data for each survey.
          switch ($variable_row_field) {
            case 'class_participation_date':
              $new_survey_records[$gid][$version]['date'] = unserialize($variable->value);
              break;
            case 'class_participation_title_value':
              $new_survey_records[$gid][$version]['title'] = unserialize($variable->value);
              break;
            case 'class_participation_max_value':
              $new_survey_records[$gid][$version]['max'] = unserialize($variable->value);
              break;
            case 'class_participation_min_value':
              $new_survey_records[$gid][$version]['min'] = unserialize($variable->value);
              break;
            case 'class_participation_starting_value':
              $new_survey_records[$gid][$version]['initial'] = unserialize($variable->value);
              break;
            case 'class_participation_survey_type':
              $new_survey_records[$gid][$version]['type'] = unserialize($variable->value);
              break;
          }
        }
      }

      foreach ($new_survey_records as $gid => $version) {
        foreach ($version as $v => $each_survey) {
          // Catch any dates that have false value and set to zero for insert.
          if ($each_survey['date'] == FALSE) {
            $each_survey['date'] = NULL;
          }
          // Insert Data into class_participation_survey table.
          db_insert('class_participation_survey')
            ->fields(array(
              'type' => $each_survey['type'],
              'gid' => $gid,
              'version' => $v,
              'title' => $each_survey['title'],
              'date' => $each_survey['date'],
              'min' => $each_survey['min'],
              'initial' => $each_survey['initial'],
              'max' => $each_survey['max'],
            ))
            ->execute();
        }
      }

      $migration_status = t('class_participation_survey table migration completed.');
    }
    else {
      $migration_status = t('class_participation_survey table already has data.  Migration cancelled.');
    }
  }
  else {
    $migration_status = t('class_participation_survey does not exist.  Migration cancelled');
  }

  return $migration_status;
}
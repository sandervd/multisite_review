<?php

/**
 * @file
 * Review for module .info files.
 */

/**
 * Performs automated review of the given module's .info file.
 */
class MultisiteReviewInfoFile extends MultisiteQaReview implements MultisiteQaReviewInterface {
  public static $extensionTypes = array('feature', 'module', 'theme');
  public static $name = 'Info file';

  /**
   * Run the test.
   */
  public function runTest() {
    $this->checkRequiredKeys();

    if ($this->extension->extension_type == 'feature') {
      $this->checkFeatureKeys();
    }
    if ($this->extension->extension_type != 'theme') {
      $this->checkTests();
    }
    $this->checkMultisiteVersion();
    $this->checkDependencies();
    $this->checkTaxonomy();
  }

  /**
   * Define a list of required keys.
   *
   * These are all specified in the custom feature development guidelines.
   * @see https://webgate.ec.europa.eu/fpfis/wikis/display/MULTISITE/Build+custom+features
   */
  public function requiredKeys() {
    $required_keys = array(
      'core',
      'description',
      'name',
      'package',
      'version',
    );
    // Features also require a 'features' key which lists the components
    // that are exported in the feature.
    if ($this->extension->extension_type == 'feature') {
      $required_keys[] = 'features';
    }
    return $required_keys;
  }

  /**
   * Check for required keys.
   */
  public function checkRequiredKeys() {
    $info = $this->extension->info;
    $name = $this->extension->name;
    foreach ($this->requiredKeys() as $key) {
      if (!array_key_exists($key, $info)) {
        $args = array(
          '@module' => $name,
          '@key' => $key,
          '@type' => $this->extension->extension_type,
        );
        $this->setMessage(dt("The @type's @key key is not set in @module.info file.", $args), 'warning');
      }
    }
  }

  /**
   * Additional check for features.
   *
   * If the module is a feature, check for some specific key/value pairs that
   * are required according to the guidelines:
   * - the minimum PHP version.
   * - the Features API version.
   */
  public function checkFeatureKeys() {
    $info = $this->extension->info;
    $required_values = array();
    $required_values['php'] = MULTISITE_REVIEW_PHP_VERSION;
    $required_values['features']['features_api'][0] = MULTISITE_REVIEW_FEATURES_API_VERSION;

    if ($missing_values = drupal_array_diff_assoc_recursive($required_values, $info)) {
      $missing_keys = implode(', ', array_keys($missing_values));
      $this->setMessage(dt('Missing required key/value pairs in the .info file: @keys', array('@keys' => $missing_keys)), 'warning');
    }
  }

  /**
   * Check if the module has at least one test.
   */
  public function checkTests() {
    $info = $this->extension->info;

    $has_tests = FALSE;
    if (array_key_exists('files', $info)) {
      foreach ($info['files'] as $file) {
        if (substr($file, -5) === '.test') {
          $has_tests = TRUE;
          break;
        }
      }
    }
    if (!$has_tests) {
      $this->setMessage(dt('There are no tests added to the files[] array.'), 'warning');
    }
  }

  /**
   * Check that there are no dependencies on the 'php' and 'menu' modules.
   */
  public function checkDependencies() {
    $info = $this->extension->info;
    foreach (array('menu', 'php') as $dependency) {
      if (!empty($info['dependencies']) && in_array($dependency, $info['dependencies'])) {
        $this->setMessage(dt('The dependency on the @dependency module should be removed.', array(
          '@dependency' => $dependency,
        )), 'warning');
      }
    }
  }

  /**
   * Check that there is no "tags" taxonomy exported.
   */
  public function checkTaxonomy() {
    $info = $this->extension->info;
    if (!empty($info['features']['taxonomy']) && in_array('tags', $info['features']['taxonomy'])) {
      $this->setMessage(dt('The "tags" vocabulary should not be exported.'));
    }
  }

  /**
   * Check the multisite platform version.
   */
  public function checkMultisiteVersion() {
    $info = $this->extension->info;
    if (!isset($info['multisite_version'])) {
      $this->setMessage(dt("The multisite version is not set."), 'warning');
      return;
    }
    if ($info['multisite_version'] != MULTISITE_REVIEW_MULTISITE_VERSION) {
      $this->setMessage(dt("The multisite version is incorrect (actual @version, expected @expected).", array(
        '@version' => $info['multisite_version'],
        '@expected' => MULTISITE_REVIEW_MULTISITE_VERSION,
      )), 'warning');
    }
  }
}

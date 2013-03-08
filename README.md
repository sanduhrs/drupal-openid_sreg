OpenID Simple Registration Extension (SREG)
===========================================
http://drupal.org/project/openid_sreg

## Description
OpenID Simple Registation is an extension to the OpenID Authentication protocol
that allows for very light-weight profile exchange. It is designed to pass eight
commonly requested pieces of information when an End User goes to register a new
account with a web service.

This module implements the complete specifications. For this purpose it extends
the Drupal core openid.module for the Consumer part and the 
openid_provider.module for the Provider part of the specification.

Using OpenID Simple Registration Extension 1.1 - Draft 1
(http://openid.net/specs/openid-simple-registration-extension-1_1-01.html)

## Requirements
Drupal 6.x

## Dependencies
* openid
* openid_provider

## Installation

1. Download OpenID Simple Registration module

   You can obtain the latest OpenID Simple Registration module release from 
   http://drupal.org/project/openid_sreg.
   
   The files are in .tar.gz format and can be extracted using most compression 
   tools.
   
   Move the resulting folder to sites/*/modules folder. See MULTISITE 
   CONFIGURATION section in Drupal core INSTALL.txt for more information.

2. Install/Setup core OpenID, OpenID Provider module

   To use the SREG extension for the OpenID Consumer activate Drupal core OpenID
   module on the ?q=admin/build/modules page.
   Configure the SREG extension on ?q=admin/settings/openid_sreg.
   
   To use the SREG extension with OpenID Provider module, donwload and install
   openid_provider.module from http://drupal.org/project/openid_provider.
   Configure the SREG extension on ?q=admin/openid/openid_sreg.

3. Apply pathc for OpenID Provider module

   For the time being, there's a patch needed, to make OpenID Provider module 
   talk to SREG extension. See the issue on http://drupal.org/node/313166.
   Follow the "Applying patches" instructions given in the Handbook pages on d.o 
   http://drupal.org/patch/apply.

## Bug reports and feature requests
1. Go to the module issue queue at http://drupal.org/project/issues/openid_sreg?status=All&categories=All
2. Click on CREATE A NEW ISSUE link.
3. Fill the form.
4. To get a status report on your request go to http://drupal.org/project/issues/user


## UPGRADING
Read more at http://drupal.org/node/250790

## License
Licensed under the GNU General Public License, GPL v2.


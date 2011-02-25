
OpenID Simple Registration Extension (SREG)
-------------------------------------------

OpenID Simple Registation is an extension to the OpenID Authentication protocol
that allows for very light-weight profile exchange. It is designed to pass eight
commonly requested pieces of information when an End User goes to register a new
account with a web service.

This module implements the complete specifications. For this purpose it extends
the Drupal core openid.module for the Consumer part and the 
openid_provider.module for the Provider part of the specification.

Using OpenID Simple Registration Extension 1.1 - Draft 1
(http://openid.net/specs/openid-simple-registration-extension-1_1-01.html)

INSTALLATION
------------

1. DOWNLOAD OPENID SIMPLE REGISTRATION MODULE

   You can obtain the latest OpenID Simple Registration module release from 
   http://drupal.org/project/openid_sreg.
   
   The files are in .tar.gz format and can be extracted using most compression 
   tools.
   
   Move the resulting folder to sites/*/modules folder. See MULTISITE 
   CONFIGURATION section in Drupal core INSTALL.txt for more information.

2. INSTALL/SETUP CORE OPENID, OPENID PROVIDER MODULE

   To use the SREG extension for the OpenID Consumer activate Drupal core OpenID
   module on the ?q=admin/build/modules page.
   Configure the SREG extension on ?q=admin/settings/openid_sreg.
   
   To use the SREG extension with OpenID Provider module, donwload and install
   openid_provider.module from http://drupal.org/project/openid_provider.
   Configure the SREG extension on ?q=admin/openid/openid_sreg.

3. APPLY PATCH FOR OPENID PROVIDER MODULE

   For the time being, there's a patch needed, to make OpenID Provider module 
   talk to SREG extension. See the issue on http://drupal.org/node/313166.
   Follow the "Applying patches" instructions given in the Handbook pages on d.o 
   http://drupal.org/patch/apply.
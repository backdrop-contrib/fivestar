// $Id$

The Five Star voting module adds a clean, attractive voting widget to nodes in Drupal 5. It features:

 * jQuery rollover effects and AJAX no-reload voting
 * Customizable star sets
 * Graceful degradation to an HTML rating form when JavaScript is turned off
 * Per-nodetype configurability
 * Support for anonymous voters
 * Spam protection to keep users from filling your DB with bogus votes
 * Easy-to-use integration with Views module for lists sorted by rating, or filtered by min/max ratings
 * A Fivestar CCK field for use in custom node types
 * An easy-to-use Form API element type for use in other modules

Fivestar was designed by Nate Haug and Jeff Eaton.

This Module Made by Robots: http://www.lullabot.com


Dependencies
------------
 * votingapi

Fivestar also provides additional features for both the CCK and Views modules.

Install
-------
Installing the Five Star voting module is simple:

1) Copy the fivestar folder to the modules folder in your installation.

2) Enable the module using Administer -> Modules (/admin/build/modules)

Note: Aggressive caching will complain that fivestar doesn't work, but it
actually causes no problems. To improve performance, the module implements
hook_init() -- and the caching advisor screen uses that as the only metric to
determine whether a module will work with the caching system. Activate it
without fear, friends -- Fivestar will continue to hum happily along.

Configuration for Simple Rating
-------------------------------

Fivestar has two completely separate modes of operation. The first is letting an
end-user rate a piece of content. The settings for this are on the content type
settings page. These settings let you expose a rating widget when viewing the
node, not editing it. Clicking on the widget registers a vote for that node, and
never anything else.

The configuration for Fivestar is spread between the content type settings page,
Fivestar site settings page, and access permissions. To configure:

1) Configure the site-wide setting for Fivestar, Administer -> Settings ->
   Fivestar.

2) Activate voting on each content type. For example, if you want Fivestar to
   appear on story nodes, use Administer -> Content Management ->
   Content Types -> Story, and check the "Enable Five Star rating" box under
   the "Five Star ratings" heading. Repeat for each content type desired.

3) Enable anonymous voting.
   If you want to allow anonymous voting, you'll need to set permissions for
   that. Use Administer -> User Management -> Access Control, and check the
   "rate content" and "view ratings" checkboxes for the roles you'd like.
   You'll find these permission items under the "fivestar module" heading.

Configuration as a CCK field / Advanced Rating
----------------------------------------------

Fivestar has extensive CCK support, which makes it so that the user is presented
with a widget to rate some node with the widget while editing a node. It does
not necessary rate the current node, nor does it rate anything if no value is
entered in the Node ID field when configuring the CCK field. The value is
saved in the node (so when you edit it it is still there), but no vote is
registered in VotingAPI without the Node ID field filled out.

An example of a situation where you might want to use the CCK fivestar field is
creating a node type called 'review'. This review type would let users rate
some particular node, and accompany their rating with a review. This could be
combined with a standard rating on the target node, so that some users could
rate the target node using the simple method, or write a complete review to
accompany their rating.

To configure a CCK field for rating a node while creating a new 'review' node:

1) Create a new node type called 'review' at Administer -> Content Management ->
Content Types. Configure the type. Do NOT set any fivestar settings on the
content type form! We don't want users to actually be able to rate the reviews
themselves!

2) Edit your new content type, then click on the "Add Field" tab while on the
content type form. Add a field called 'rating' to your new type, make it of type
Fivestar Rating with the Stars radio button.

3) Configure the rating widget to your liking. Most field have help text which
explain their purpose. The Node ID field is the most important field on the page
which determines exactly what node will receive the value of the rating. In a
realy simple case, you could just enter the value 10 to always rate on the same
node with nid = 10. Usually you'll need to enter PHP code to dynamically select
what node you want to rate.

A common scenario is using fivestar with nodecomments to make reviews. If using
nodecomments a separate checkbox appears the Node ID field to allow you easily
select the nodecomment parent as the target of the vote.

Save your field. Now when making new nodes of type 'review', the user will
select a star that will register a vote on the value of the Node ID field.

Views Integration
-----------------
Fivestar depends on the views integration provided by VotinAPI, but adds some
special features to make it work specifically with Fivestar. To display Fivestar
ratings in a view, select the "VotingAPI percent vote result" from the list of
available Fields. This will display the average vote for nodes. Then choose
"Fivestar rating" from the Handler options for the field and the averages will
be displayed as Fivestar ratings.

Fivestar also provides handling for the display of Fivestar CCK fields, they are
in the Field list under "Fivestar Rating: [Field name]".


Contributing
------------
Have a sweet set of stars you'd like to contribute to the Fivestar module?
Post them to the Fivestar issue queue: http://drupal.org/project/issues/fivestar

Support
-------
If you experience a problem with fivestar or have a problem, file a
request or issue on the fivestar queue at
http://drupal.org/project/issues/fivestar. DO NOT POST IN THE FORUMS. Posting in
the issue queues is a direct line of communication with the module authors.
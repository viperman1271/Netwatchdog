import { checkTokenAndRedirect } from './check.js'

document.addEventListener('DOMContentLoaded', function() {
    checkTokenAndRedirect();
});

function redirectToLogin() {
    if(window.location.pathname !== "/login.html") {
        window.location.href = '/login.html';
    }
}
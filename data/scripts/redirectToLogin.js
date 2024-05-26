export function redirectToLogin() {
    if(window.location.pathname !== "/login.html") {
        window.location.href = '/login.html';
    }
}
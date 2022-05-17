// Test JS code sample (not part of C coding submission).
// https://developer.mozilla.org/en-US/docs/Glossary/IIFE
for (var i = 0; i < 2; i++) {
    const button = document.createElement('button');
    button.innerText = 'Button ' + i;
    button.onclick = (function (copyOfI) {
        return () => {
            console.log(copyOfI);
        };
    })(i);
    document.body.appendChild(button);
}
console.log(i); // 2